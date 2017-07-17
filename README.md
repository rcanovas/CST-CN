## CST-CN

The Compressed Suffix Tree CN (CST_CN) implementation contains the C++11 
codes associated with the following reference: 
R. Canovas and G. Navarro. "Practical Compressed Suffix Trees". In Proc. SEA, 94--105, 2010.


This work includes the following data structures:

- cst_cn: The compressed suffix tree CN data structure. It receives as template parameters the CSA, LCP, and NPR support data structures to be used. Also receives the size of the blocks and small block used for the NPR support structure.
- npr_support_cn: A class to represent the NPR (next/previuos smaller value and range minimum query) based on Canovas and Navarro work. This implementation receives a parameter indicating the size of the "block" used.
- npr_support_cnr: A class to represent the NPR (next/previuos smaller value and range minimum query) representing a reduced version of npr_support_cn. This implementation receives a parameter indicating the size of the "block" and "small block" used.


The createIndex test shows examples of how to create the data structure using user defined and default templates parameters. 

## Compile

To be able to compile the codes: 
- Install sdsl-lite. Follow the installation guide here: (https://github.com/simongog/sdsl-lite)
- Modify the location of the sdsl library in the CMakeLists.txt if necessary.
- Need cmake version 3.3 or higher.
- Create the build folder and run: 
	- cmake ..
	- make


## Methods

-[createCST] :

	Use: ./createCST <file_name> <opt> 
      		<file_name>: Name of the file to be use to create the required data structure 
		<opt> : 
			-t temporal_folder:  String containing the name of the temporal folder used. Default /tmp
		  	-o output_name:  String containing the name of the output file. Default file_name.cst_type
			-w Index_type. Default = 0
        		    ---+--------------------
        		     0 | CST_CN with NPR-CN
        		     1 | CST_CN with NPR-CNR
        		     2 | CST_SADA   (from sdsl-lite)
       			     3 | CST_SCT3   (from sdsl-lite)
        		-c suffix array: CSA used within the CST chosen. Default = 0
        		    ---+--------------------"
        		     0 | CSA_WT
        		     1 | CSA_SADA
        		-l lcp array: LCP used within the CST chosen. Default = 0
   			    ---+--------------------
        		     0 | LCP_DAC
        		     1 | LCP_SUPPORT_SADA (for CST-CN) and LCP_SUPPORT_TREE2 (for CST-SCT3 and CST_SADA)
        		-b block_size:  Block size for NPR of CN and CNR (values accepted in this test: 32, 16, 8). Default = 32 
        		-s small block_size:  Small Block size for NPR of CNR (values accepted in this test: 8, 4). Default = 8 

          	output:  <output_name>.cst_type

		Example: ./createCST file -w 0 -c 0 -l 0 -b 16
		output:  file.cst_cn_wt_dac_16
        

-[testOps] :

	Use: ./testOps <cst_file> <opt>
		<cst_file_name>: Name of the file containing the CST data structure
		<opt> (index details are requiered): 
			-w Index_type. Default = 0
        		    ---+--------------------
        		     0 | CST_CN with NPR-CN
        		     1 | CST_CN with NPR-CNR
        		     2 | CST_SADA   (from sdsl-lite)
       			     3 | CST_SCT3   (from sdsl-lite)
        		-c suffix array: CSA used within the CST chosen. Default = 0
        		    ---+--------------------"
        		     0 | CSA_WT
        		     1 | CSA_SADA
        		-l lcp array: LCP used within the CST chosen. Default = 0
   			    ---+--------------------
        		     0 | LCP_DAC
        		     1 | LCP_SUPPORT_SADA (for CST-CN) and LCP_SUPPORT_TREE2 (for CST-SCT3 and CST_SADA)
        		-b block_size:  Block size for NPR of CN and CNR (values accepted in this test: 32, 16, 8). Default = 32 
        		-s small block_size:  Small Block size for NPR of CNR (values accepted in this test: 8, 4). Default = 8 
	

		output:  Times per operation

		Example: ./testSearch ./data/file.acat 2 ./sample/file_sample  
		output:  
		        index: cst_cn<csa_sada, lcp_dac, 16>
			CSA uses: 5.33172n bits
			LCP uses: 6.38106n bits
			NPR uses: 2.20001n bits
			Size Text: 209715201
			Size in bytes: 364715365 bytes
			Size in bits: 13.9128n bits
			Sample V1 size: 161638
			Parent: 815.168 nanosec
			Depth: 820.931 nanosec
			First Child: 815.722 nanosec
			Sibling: 1055.14 nanosec
			Node-Depth: 6341.75 nanosec
			Child: 11865.9 nanosec
			Sample V2 size: 77831
			Slink: 1839.64 nanosec
			Sample V3 size: 200000
			LCA: 1024.34 nanosec
			

For more information please refer to the paper "Practical Compressed Suffix Trees". In Proc. SEA, 94--105, 2010. Additional 
information of the NPR support data structure and experimental results can be found in the CTS_additonal_data.pdf file.
	
			
Note: These codes assume that the computer used has enough RAM to read and store the complete input.
