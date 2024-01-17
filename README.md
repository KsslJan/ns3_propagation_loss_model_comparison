# ns3_propagation_loss_model_comparison

Publication of source code for a comparison of the following propagation loss models regarding received signal strength and throughput over varying distance:
- FriisPropagationLossModel
- FixedRssPropagationLossModel
- TwoRayGroundPropagationLossModel
- ThreeLogDistancePropagationLossModel
- NakagamiPropagationLossModel

This publication has been made regarding a master's course. 

## Usage
1. Download and install ns3-3.39
2. run propagation_loss_model_comparison.cc with ./ns3 with the "model" parameter to select model
3. files will be put in the data folder
4. run create_diagrams.py to create diagrams from the models in the data folder 
