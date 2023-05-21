# HW3
## To the base directory (or other versions)
$ cd filter_base/stratus

## Tools
$ source /usr/cadtool/user_setup/01-cadence_license_set.cshset
$ source /usr/cadtool/user_setup/03-stratus.csh
$ source /usr/cadtool/user_setup/03-xcelium.csh
$ setenv STRATUS_EXAMPLE /usr/cad/cadence/STRATUS/cur/share/stratus/collateral/examples

## For the behavioral simulation
$ make sim_B 
## For the verilog simulation for HLS config
$ make sim_V_BASIC
