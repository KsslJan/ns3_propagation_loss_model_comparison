
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ssid.h"
#include "ns3/adhoc-wifi-mac.h"
#include "ns3/wifi-module.h"
#include "ns3/wifi-standards.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/callback.h"
#include "ns3/ipv4-flow-classifier.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/callback.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ModelTesting");

int distance = 0;
double signalStrengthDbm; 

/**
 * Monitor sniffer Rx trace
 *
 * \param packet The sensed packet.
 * \param channelFreqMhz The channel frequency [MHz].
 * \param txVector The Tx vector.
 * \param aMpdu The aMPDU.
 * \param signalNoise The signal and noise dBm.
 * \param staId The STA ID.
 */
void
MonitorSniffRx(Ptr<const Packet> packet,
               uint16_t channelFreqMhz,
               WifiTxVector txVector,
               MpduInfo aMpdu,
               SignalNoiseDbm signalNoise,
               uint16_t staId)
{
    signalStrengthDbm = signalNoise.signal;
    // NS_LOG_UNCOND("distance: " << distance << " | signal strength: " << signalNoise.signal);
}

int
main(int argc, char* argv[])
{
    CommandLine cmd(__FILE__);
    int modelNumber = 0;
    cmd.AddValue("model", "number for model which should be executed; 0=friis, 1=fixedRss, 2=threeLogDistance, 3=twoRayGround, 4=nakagami", modelNumber);
    cmd.Parse(argc, argv);

    double simulationTime = 50.0;

    std::string modelName = "";
    switch (modelNumber)
    {
    case 0:
        modelName = "FriisPropagationLossModel";
        break;
    case 1:
        modelName = "FixedRssLossModel";
        break;
    case 2:
        modelName = "ThreeLogDistancePropagationLossModel";
        break;
    case 3:
        modelName = "TwoRayGroundPropagationLossModel";
        break;
    case 4:
        modelName = "NakagamiPropagationLossModel";
        break;
    default:
        throw std::invalid_argument("model number needs to be between 0 and 4");
    }
    std::cout << "Running with " << modelName << " with runtime of " << simulationTime << " seconds" << std::endl;

    std::string signalStrengthFile = "scratch/data/" + modelName + "_rss.txt";
    std::string throughputFile = "scratch/data/" + modelName + "_throughput.txt";
    std::string distanceFile = "scratch/data/" + modelName + "_distance.txt";
    FILE *signalStrengthFilePointer = fopen(signalStrengthFile.c_str(), "w");
    FILE *throughputFilePointer = fopen(throughputFile.c_str(), "w");
    FILE *distanceFilePointer = fopen(distanceFile.c_str(), "w");

    if (signalStrengthFilePointer == NULL || throughputFilePointer == NULL || distanceFilePointer == NULL) {
        printf("Error opening file.\n");
        return 1;
    }

    for (double dist = 1.0; dist <= 250.0; dist+=1.0)
    {
        distance = dist;
    
        //2 Wifi nodes
        NodeContainer wifiNodes;
        wifiNodes.Create(2);
        Ptr<Node> constNode = wifiNodes.Get(0);
        Ptr<Node> mobilityNode = wifiNodes.Get(1);

        YansWifiPhyHelper wifiPhyHelper;
        //omnidirectional antenna
        // 1 dBi 
        wifiPhyHelper.Set("RxGain", DoubleValue(1));
        wifiPhyHelper.Set("TxGain", DoubleValue(1));
        //5GHz
        wifiPhyHelper.Set("ChannelSettings", StringValue("{0, 40, BAND_5GHZ, 0}"));
        
        //10dBm transmission power
        wifiPhyHelper.Set("TxPowerStart", DoubleValue(10.0));
        wifiPhyHelper.Set("TxPowerEnd", DoubleValue(10.0));

        YansWifiChannelHelper wifiChannelHelper;
        wifiChannelHelper.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");

        //FriisPropagationLossModel
        //FixedRssLossModel
        //ThreeLogDistancePropagationLossModel
        //TwoRayGroundPropagationLossModel
        //Nakagami
        double z = 0.0;
        switch (modelNumber){
        case 0:
            { // FriisPropagationLossModel
                std::cout << "FriisPropagationLossModel applied" << std::endl;
                wifiChannelHelper.AddPropagationLoss("ns3::FriisPropagationLossModel");
                break;
            }
        case 1: // wifi-simple-adhoc.cc
            {
                std::cout << "FixedRssLossModel applied" << std::endl;
                double rssValue = -80; // in dbm
                wifiChannelHelper.AddPropagationLoss("ns3::FixedRssLossModel", "Rss", DoubleValue(rssValue));
                break;
            }
        case 2: // wifi-clear-channel-cmu.cc
            {
                std::cout << "ThreeLogDistancePropagationLossModel applied" << std::endl;
                wifiChannelHelper.AddPropagationLoss("ns3::ThreeLogDistancePropagationLossModel");
                break;
            }
        case 3:
            {                
                std::cout << "TwoRayGroundPropagationLossModel applied" << std::endl;
                wifiChannelHelper.AddPropagationLoss("ns3::TwoRayGroundPropagationLossModel", "HeightAboveZ", DoubleValue(1)); 
                // not working without changing the height of the antenna above the nodes z-coordinate
                // to keep the same distance as the other experiments, z-height for the mobility model also needs to be updated
                z = 1.0;
                break;
            }
        case 4:
            {                
                std::cout << "NakagamiPropagationLossModel applied" << std::endl;
                wifiChannelHelper.AddPropagationLoss("ns3::NakagamiPropagationLossModel");
                break;
            }
        default:
            {
                throw std::invalid_argument("model number needs to be between 0 and 4");
            }
        }
        wifiPhyHelper.SetChannel(wifiChannelHelper.Create());


        WifiMacHelper wifiMac;
        //Adhoc Wi-fi MAC
        wifiMac.SetType("ns3::AdhocWifiMac");

        WifiHelper wifiHelper;
        //IEEE802.11n
        wifiHelper.SetStandard(WIFI_STANDARD_80211n);

        NetDeviceContainer wifiDevices = wifiHelper.Install(wifiPhyHelper, wifiMac, wifiNodes);
        
        //IP
        InternetStackHelper stack;
        stack.Install(wifiNodes);
    
        Ipv4AddressHelper ipv4Helper;
        ipv4Helper.SetBase("10.1.1.0", "255.255.255.0");
        
        Ipv4InterfaceContainer ipv4Interface;
        ipv4Interface = ipv4Helper.Assign(wifiDevices);
        
        Ipv4GlobalRoutingHelper::PopulateRoutingTables();
        //UDP
        uint32_t port = 9;
        UdpServerHelper udpServerHelper(port);
        ApplicationContainer serverApps = udpServerHelper.Install(constNode);
        serverApps.Start(Seconds(1.0));
        serverApps.Stop(Seconds(simulationTime));


        uint32_t payloadSize = 1450;

        UdpClientHelper udpClientHelper(ipv4Interface.GetAddress(0), port);
        udpClientHelper.SetAttribute("MaxPackets", UintegerValue(4294967295U));
        // Time interval = payloadSize / datarate
        // approximately 0.0001547
        udpClientHelper.SetAttribute("Interval", TimeValue(Time("0.0001547"))); // packets/s
        udpClientHelper.SetAttribute("PacketSize", UintegerValue(payloadSize));
        ApplicationContainer clientApps = udpClientHelper.Install(mobilityNode);
        clientApps.Start(Seconds(2.0));
        clientApps.Stop(Seconds(simulationTime));

        Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
        positionAlloc->Add(Vector(0.0, 0.0, z));
        positionAlloc->Add(Vector(dist, 0.0, z));

        MobilityHelper mobility;
        mobility.SetPositionAllocator(positionAlloc);
        mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
        mobility.Install(wifiNodes);

        //needed RESULT:
        //signal strength at physical layer of wifi card
        //throughput

        // FlowMonitorHelper flowmon;
        // Ptr<FlowMonitor> monitor = flowmon.InstallAll();

        //signal strength in dbm
        Config::ConnectWithoutContext("/NodeList/0/DeviceList/*/Phy/MonitorSnifferRx", MakeCallback(&MonitorSniffRx));

        Simulator::Stop(Seconds(simulationTime));
        Simulator::Run();
            
        // monitor->CheckForLostPackets();
        // Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier());
        // FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats();

        uint64_t totalPacketsThrough = DynamicCast<UdpServer>(serverApps.Get(0))->GetReceived();
        double throughput = totalPacketsThrough * payloadSize * 8 / (simulationTime * 1000000.0);

        // double throughput = 0; 
        // for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator it = stats.begin();
        //     it != stats.end();
        //     ++it)
        // {
        //     Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(it->first);

        //     // in mbps
        //     throughput = it->second.rxBytes * 8.0 / (it->second.timeLastRxPacket.GetSeconds() - it->second.timeFirstTxPacket.GetSeconds())/1024/1024;
        //     std::cout << "source: " << t.sourceAddress
        //                             << "; dest: " << t.destinationAddress
        //                             << "; dist: " << dist
        //                             << "; dbm: " << signalStrengthDbm
        //                             << "; tx " << it->second.txBytes 
        //                             << "; rx " << it->second.rxBytes
        //                             << "; diff: " << (it->second.txBytes - it->second.rxBytes)
        //                             << "; mbps: " << throughput
        //                             << std::endl;
                                      
        // }

        // std::string tr_name(std::to_string(modelNumber) + "_dist_");
        // flowmon.SerializeToXmlFile(tr_name + std::to_string(dist).substr(0,5) + ".flowmon", false, false);
        
        std::cout << "dist: " << dist
                  << "; rss: " << signalStrengthDbm
                  << "; throughput: " << throughput
                  << std::endl;

        // for ploting
        if(abs(throughput) < 0.0001){
            throughput = 0.0;
        }
        fprintf(signalStrengthFilePointer,"%f;", signalStrengthDbm);
        fprintf(throughputFilePointer,"%f;", throughput);
        fprintf(distanceFilePointer,"%f;", dist);

        Simulator::Destroy();
    }
    fclose(signalStrengthFilePointer);
    fclose(throughputFilePointer);
    fclose(distanceFilePointer);
    
    std::cout << "Wrote results into " << modelName << std::endl;
    return 0;     
}