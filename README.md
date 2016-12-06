ICNP 2016: A Sorted Partitioning Approach to High-speed and Fast-update OpenFlow Classification

Tested on Ubuntu 14.04.4 LTS. 

Requirement:
g++ at least version 4.9.

Installation: make

How to run the simulator: ./main [options]

select filter: f="fw1_seed_1.rules" 

select modes:  m="Classification", "Update", or "Validate" (Default: classification)

select output path and filename.csv: o="Output/64k_fw1_seed_1.csv"

select classifiers: c="PartitionSort,PriorityTuple". It is possible to run multiple classifiers. (Classifiers: "PartitionSort", "PriorityTuple", "HyperCuts", "HyperSplit", "All") 

Try now (no space between = sign):

./main f="fw1_seed_1.rules" c="PartitionSort,PriorityTuple" m="Classification" o="Output/64k_fw1_seed_1.csv"

Note: 

In the code, we set to run repeatedly 10 times during classification or updates and then report the average. 

You can find rulesets that we used in experiment in Rulesets folder.

You can customize your own classifier. See BruteForce.h as an example. Note that deletion function requires the strict ordering of rule rearrangement as in BruteForce.h. Make sure that you test correctness by running Validation mode. 

Please contact me if you have any question. 

Sorrachai Yingchareonthawornchai

Michigan State University

yingchar[at]cse.msu.edu 

Nov 21, 2016


Acknowledgement:
MITree implementation is developed based on the raw red-black tree implementation from

http://web.mit.edu/~emin/Desktop/ref_to_emin/www.old/source_code/red_black_tree/index.html

[Online; Jan, 2016]


