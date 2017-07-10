/* cstds - compressed suffix tree data structure
 * Copyright (C)2016-2017 Rodrigo Canovas
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/ .
 * */


#include <iostream>
#include <sdsl/suffix_trees.hpp>
#include "../include/cst_cn.h"

using namespace std;

template<class idx_type>
void
create_index(string file, string tmp_dir, string out_file) {
    using timer = std::chrono::high_resolution_clock;
    auto start = timer::now();
    idx_type idx;
    string id = sdsl::util::basename(file);
    sdsl::cache_config config(true, tmp_dir, id); //true -->erase tmp
    construct(idx, file, config, 1);
    ofstream out(out_file);
    std::cout << "Size Text: " << idx.size() << std::endl;
    uint64_t bytes_size = idx.serialize(out);
    auto stop = timer::now();
    auto elapsed = stop - start;
    cout << "Construction time: " << ((chrono::duration_cast<chrono::seconds>(elapsed).count() * 1.0)) << " seconds" << endl;
	out.close();
    std::cout << "Size in bits: " << (bytes_size * 8.0 / idx.size()) << "n bits" << std::endl;
}

int main(int argc, char* argv[]) {

    if(argc < 2) {
        cout << "Usage: " << argv[0] << " file_name <opt>" << endl;
        cout << "opt: " << endl;
        cout << "-t temporal_folder:  String containing the name of the temporal folder used. Default /tmp" << endl;
        cout << "-o output_name:  String containing the name of the output file. Default file_name.cst_type" << endl;
        cout << "-w Index_type. Default = 0" << endl;
        cout << "    ---+--------------------" << endl;
        cout << "     0 | CST_CN with NPR-CN" << endl;
        cout << "     1 | CST_CN with NPR-CNR" << endl;
        cout << "     2 | CST_SADA" << endl;
        cout << "     3 | CST_SCT3" << endl;
        cout << "-c suffix array: CSA used within the CST chosen. Default = 0 " << endl;
        cout << "    ---+--------------------" << endl;
        cout << "     0 | CSA_WT" << endl;
        cout << "     1 | CSA_SADA" << endl;
        cout << "-l lcp array: LCP used within the CST chosen. Default = 0 " << endl;
        cout << "    ---+--------------------" << endl;
        cout << "     0 | LCP_DAC" << endl;
        cout << "     1 | LCP_SUPPORT_SADA (for CST-CN) and LCP_SUPPORT_TREE2 (for CST-SCT3 and CST_SADA)" << endl;
        cout << "-b block_size:  Block size for NPR of CN and CNR (values accepted in this test: 32, 16, 8). Default = 32 " << endl;
        cout << "-s small block_size:  Small Block size for NPR of CNR (values accepted in this test: 8, 4). Default = 8 " << endl;
        return 1;
    }

    string file = argv[1];
    string out_file = file;
    string tmp_dir = "/tmp";
    int w = 0, c = 0, l = 0, b = 32, s = 8;

    int o;
    while((o = getopt (argc, argv, "o:w:t:c:l:b:s:")) != -1){
        switch (o) {
            case 'o': out_file = optarg;  break;
            case 'w': w = atoi(optarg); break;
            case 't': tmp_dir = optarg; break;
            case 'c': c = atoi(optarg); break;
            case 'l': l = atoi(optarg); break;
            case 'b': b = atoi(optarg); break;
            case 's': s = atoi(optarg); break;
            case '?':
                if(optopt == 'o' || optopt == 'w' || optopt == 't' ||
                         optopt == 'c' || optopt == 'l' || optopt == 'b' ||  optopt == 's')
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                else
                    fprintf(stderr,"Unknown option character `\\x%x'.\n",	optopt);
                return 1;
            default:  abort ();
        }

    }


    switch (w) {
        case 0:  //CST-CN with NPR-CN
            switch (c) {
                case 0: //CSA-WT
                    if (l == 0) { //LCP-DAC
                        switch (b) {
                            case 8:
                                cout << "index: cst_cn<csa_wt, lcp_dac, 8>" << std::endl;
                                out_file += ".cst_cn_wt_dac_8";
                                create_index<cstds::cst_cn<csa_wt<>, lcp_dac<>,
                                                           cstds::npr_support_cn, 8> >(file, tmp_dir, out_file);
                                break;
                            case 16:
                                cout << "index: cst_cn<csa_wt, lcp_dac, 16>" << std::endl;
                                out_file += ".cst_cn_wt_dac_16";
                                create_index<cstds::cst_cn<csa_wt<>, lcp_dac<>,
                                                           cstds::npr_support_cn, 16> >(file, tmp_dir, out_file);
                                break;
                            case 32:
                                cout << "index: cst_cn<csa_wt, lcp_dac, 32>" << std::endl;
                                out_file += ".cst_cn_wt_dac_32";
                                create_index<cstds::cst_cn<csa_wt<>, lcp_dac<>,
                                                           cstds::npr_support_cn, 32> >(file, tmp_dir, out_file);
                                break;
                            default:
                                cout << "Error: the -b option must be 8, 16, or 32" << endl;
                        }
                    }
                    else if (l == 1) { //LCP-SUPPORT-SADA
                        switch (b) {
                            case 8:
                                cout << "index: cst_cn<csa_wt, lcp_support_sada, 8>" << std::endl;
                                out_file += ".cst_cn_wt_sa_8";
                                create_index<cstds::cst_cn<csa_wt<>, lcp_support_sada<>,
                                                           cstds::npr_support_cn, 8> >(file, tmp_dir, out_file);
                                break;
                            case 16:
                                cout << "index: cst_cn<csa_wt, lcp_support_sada, 16>" << std::endl;
                                out_file += ".cst_cn_wt_sa_16";
                                create_index<cstds::cst_cn<csa_wt<>, lcp_support_sada<>,
                                                           cstds::npr_support_cn, 16> >(file, tmp_dir, out_file);
                                break;
                            case 32:
                                cout << "index: cst_cn<csa_wt, lcp_support_sada, 32>" << std::endl;
                                out_file += ".cst_cn_wt_sa_32";
                                create_index<cstds::cst_cn<csa_wt<>, lcp_support_sada<>,
                                                           cstds::npr_support_cn, 32> >(file, tmp_dir, out_file);
                                break;
                            default:
                                cout << "Error: the -b option must be 8, 16, or 32" << endl;
                        }
                    }
                    else
                        cout << "Error: the -l option must be in [0,1]" << endl;
                    break;
                case 1: // CSA-SADA
                    if (l == 0) {
                        switch (b) { //LCP-DAC
                            case 8:
                                cout << "index: cst_cn<csa_sada, lcp_dac, 8>" << std::endl;
                                out_file += ".cst_cn_sa_dac_8";
                                create_index<cstds::cst_cn<csa_sada<>, lcp_dac<>,
                                                           cstds::npr_support_cn, 8> >(file, tmp_dir, out_file);
                                break;
                            case 16:
                                cout << "index: cst_cn<csa_sada, lcp_dac, 16>" << std::endl;
                                out_file += ".cst_cn_sa_dac_16";
                                create_index<cstds::cst_cn<csa_sada<>, lcp_dac<>,
                                                           cstds::npr_support_cn, 16> >(file, tmp_dir, out_file);
                                break;
                            case 32:
                                cout << "index: cst_cn<csa_sada, lcp_dac, 32>" << std::endl;
                                out_file += ".cst_cn_sa_dac_32";
                                create_index<cstds::cst_cn<csa_sada<>, lcp_dac<>,
                                                           cstds::npr_support_cn, 32> >(file, tmp_dir, out_file);
                                break;
                            default:
                                cout << "Error: the -b option must be 8, 16, or 32" << endl;
                        }
                    }
                    else if (l == 1) { //LCP-SUPPORT-SADA
                        switch (b) {
                            case 8:
                                cout << "index: cst_cn<csa_sada, lcp_support_sada, 8>" << std::endl;
                                out_file += ".cst_cn_sa_sa_8";
                                create_index<cstds::cst_cn<csa_sada<>, lcp_support_sada<>,
                                                           cstds::npr_support_cn, 8> >(file, tmp_dir, out_file);
                                break;
                            case 16:
                                cout << "index: cst_cn<csa_sada, lcp_support_sada, 16>" << std::endl;
                                out_file += ".cst_cn_sa_sa_16";
                                create_index<cstds::cst_cn<csa_sada<sdsl::enc_vector<>, 32, 32>, lcp_support_sada<>,
                                                           cstds::npr_support_cn, 16> >(file, tmp_dir, out_file);
                                break;
                            case 32:
                                cout << "index: cst_cn<csa_sada, lcp_support_sada, 32>" << std::endl;
                                out_file += ".cst_cn_sa_sa_32";
                                create_index<cstds::cst_cn<csa_sada<sdsl::enc_vector<>, 32, 32>, lcp_support_sada<>,
                                                           cstds::npr_support_cn, 32> >(file, tmp_dir, out_file);
                                break;
                            default:
                                cout << "Error: the -b option must be 8, 16, or 32" << endl;
                        }
                    }
                    else
                        cout << "Error: the -l option must be in [0,1]" << endl;

                    break;
                default:
                    cout << "Error: the -c option must be in [0,1]" << endl;
            }
            break;
        case 1: //CST-CN with NPR-CNR
            switch (c) {
                case 0: //CSA-WT
                    if (l == 0) { //LCP-DAC
                        switch (b) {
                            case 8:
                                if (s == 4) {
                                    cout << "index: cst_cn<csa_wt, lcp_dac, 8, 4>" << std::endl;
                                    out_file += ".cst_cn_wt_dac_8_4";
                                    create_index<cstds::cst_cn<csa_wt<>, lcp_dac<>,
                                                           cstds::npr_support_cnr, 8, 4> >(file, tmp_dir, out_file);
                                }
                                else if (s == 8) {
                                    cout << "index: cst_cn<csa_wt, lcp_dac, 8, 8>" << std::endl;
                                    out_file += ".cst_cn_wt_dac_8_8";
                                    create_index<cstds::cst_cn<csa_wt<>, lcp_dac<>,
                                                           cstds::npr_support_cnr, 8, 8> >(file, tmp_dir, out_file);
                                }
                                else
                                    cout << "Error: the -s option must be 4, 8" << endl;
                                break;
                            case 16:
                                if (s == 4) {
                                    cout << "index: cst_cn<csa_wt, lcp_dac, 16, 4>" << std::endl;
                                    out_file += ".cst_cn_wt_dac_16_4";
                                    create_index<cstds::cst_cn<csa_wt<>, lcp_dac<>,
                                                           cstds::npr_support_cnr, 16, 4> >(file, tmp_dir, out_file);
                                }
                                else if (s == 8) {
                                    cout << "index: cst_cn<csa_wt, lcp_dac, 16, 8>" << std::endl;
                                    out_file += ".cst_cn_wt_dac_16_8";
                                    create_index<cstds::cst_cn<csa_wt<>, lcp_dac<>,
                                                           cstds::npr_support_cnr, 16, 8> >(file, tmp_dir, out_file);
                                }
                                else
                                    cout << "Error: the -s option must be 4, 8" << endl;
                                break;
                            case 32:
                                if (s == 4) {
                                    cout << "index: cst_cn<csa_wt, lcp_dac, 32, 4>" << std::endl;
                                    out_file += ".cst_cn_wt_dac_32_4";
                                    create_index<cstds::cst_cn<csa_wt<>, lcp_dac<>,
                                                           cstds::npr_support_cnr, 32, 4> >(file, tmp_dir, out_file);
                                }
                                else if (s == 8) {
                                    cout << "index: cst_cn<csa_wt, lcp_dac, 32, 8>" << std::endl;
                                    out_file += ".cst_cn_wt_dac_32_8";
                                    create_index<cstds::cst_cn<csa_wt<>, lcp_dac<>,
                                                           cstds::npr_support_cnr, 32, 8> >(file, tmp_dir, out_file);
                                }
                                else
                                    cout << "Error: the -s option must be 4, 8" << endl;
                                break;
                            default:
                                cout << "Error: the -b option must be 8, 16, or 32" << endl;
                        }
                    }
                    else if (l == 1) { //LCP-SUPPORT-SADA
                        switch (b) {
                            case 8:
                                if (s == 4) {
                                    cout << "index: cst_cn<csa_wt, lcp_sada, 8, 4>" << std::endl;
                                    out_file += ".cst_cn_wt_sa_8_4";
                                    create_index<cstds::cst_cn<csa_wt<>, lcp_support_sada<>,
                                                           cstds::npr_support_cnr, 8, 4> >(file, tmp_dir, out_file);
                                }
                                else if (s == 8) {
                                    cout << "index: cst_cn<csa_wt, lcp_sada, 8, 8>" << std::endl;
                                    out_file += ".cst_cn_wt_sa_8_8";
                                    create_index<cstds::cst_cn<csa_wt<>, lcp_support_sada<>,
                                                           cstds::npr_support_cnr, 8, 8> >(file, tmp_dir, out_file);
                                }
                                else
                                    cout << "Error: the -s option must be 4, 8" << endl;
                                break;
                            case 16:
                                if (s == 4) {
                                    cout << "index: cst_cn<csa_wt, lcp_sada, 16, 4>" << std::endl;
                                    out_file += ".cst_cn_wt_sa_16_4";
                                    create_index<cstds::cst_cn<csa_wt<>, lcp_support_sada<>,
                                                           cstds::npr_support_cnr, 16, 4> >(file, tmp_dir, out_file);
                                }
                                else if (s == 8) {
                                    cout << "index: cst_cn<csa_wt, lcp_sada, 16, 8>" << std::endl;
                                    out_file += ".cst_cn_wt_sa_16_8";
                                    create_index<cstds::cst_cn<csa_wt<>, lcp_support_sada<>,
                                                           cstds::npr_support_cnr, 16, 8> >(file, tmp_dir, out_file);
                                }
                                else
                                    cout << "Error: the -s option must be 4, 8" << endl;
                                break;
                            case 32:
                                if (s == 4) {
                                    cout << "index: cst_cn<csa_wt, lcp_sada, 32, 4>" << std::endl;
                                    out_file += ".cst_cn_wt_sa_32_4";
                                    create_index<cstds::cst_cn<csa_wt<>, lcp_support_sada<>,
                                                           cstds::npr_support_cnr, 32, 4> >(file, tmp_dir, out_file);
                                }
                                else if (s == 8) {
                                    cout << "index: cst_cn<csa_wt, lcp_sada, 32, 8>" << std::endl;
                                    out_file += ".cst_cn_wt_sa_32_8";
                                    create_index<cstds::cst_cn<csa_wt<>, lcp_support_sada<>,
                                                           cstds::npr_support_cnr, 32, 8> >(file, tmp_dir, out_file);
                                }
                                else
                                    cout << "Error: the -s option must be 4, 8" << endl;
                                break;
                            default:
                                cout << "Error: the -b option must be 8, 16, or 32" << endl;
                        }
                    }
                    else
                        cout << "Error: the -l option must be in [0,1]" << endl;
                    break;
                case 1: // CSA-SADA
                    if (l == 0) {
                        switch (b) { //LCP-DAC
                            case 8:
                                if (s == 4) {
                                    cout << "index: cst_cn<csa_sada, lcp_dac, 8, 4>" << std::endl;
                                    out_file += ".cst_cn_sa_dac_8_4";
                                    create_index<cstds::cst_cn<csa_sada<>, lcp_dac<>,
                                                           cstds::npr_support_cnr, 8, 4> >(file, tmp_dir, out_file);
                                }
                                else if (s == 8) {
                                    cout << "index: cst_cn<csa_sada, lcp_dac, 8, 8>" << std::endl;
                                    out_file += ".cst_cn_sa_dac_8_8";
                                    create_index<cstds::cst_cn<csa_sada<>, lcp_dac<>,
                                                           cstds::npr_support_cnr, 8, 8> >(file, tmp_dir, out_file);
                                }
                                else
                                    cout << "Error: the -s option must be 4, 8" << endl;
                                break;
                            case 16:
                                if (s == 4) {
                                    cout << "index: cst_cn<csa_sada, lcp_dac, 16, 4>" << std::endl;
                                    out_file += ".cst_cn_sa_dac_16_4";
                                    create_index<cstds::cst_cn<csa_sada<>, lcp_dac<>,
                                                           cstds::npr_support_cnr, 16, 4> >(file, tmp_dir, out_file);
                                }
                                else if (s == 8) {
                                    cout << "index: cst_cn<csa_sada, lcp_dac, 16, 8>" << std::endl;
                                    out_file += ".cst_cn_sa_dac_16_8";
                                    create_index<cstds::cst_cn<csa_sada<>, lcp_dac<>,
                                                           cstds::npr_support_cnr, 16, 8> >(file, tmp_dir, out_file);
                                }
                                else
                                    cout << "Error: the -s option must be 4, 8" << endl;
                                break;
                            case 32:
                                if (s == 4) {
                                    cout << "index: cst_cn<csa_sada, lcp_dac, 32, 4>" << std::endl;
                                    out_file += ".cst_cn_sa_dac_32_4";
                                    create_index<cstds::cst_cn<csa_sada<>, lcp_dac<>,
                                                           cstds::npr_support_cnr, 32, 4> >(file, tmp_dir, out_file);
                                }
                                else if (s == 8) {
                                    cout << "index: cst_cn<csa_sada, lcp_dac, 32, 8>" << std::endl;
                                    out_file += ".cst_cn_sa_dac_32_8";
                                    create_index<cstds::cst_cn<csa_sada<>, lcp_dac<>,
                                                           cstds::npr_support_cnr, 32, 8> >(file, tmp_dir, out_file);
                                }
                                else
                                    cout << "Error: the -s option must be 4, 8" << endl;
                                break;
                            default:
                                cout << "Error: the -b option must be 8, 16, or 32" << endl;
                        }
                    }
                    else if (l == 1) { //LCP-SUPPORT-SADA
                        switch (b) {
                            case 8:
                                if (s == 4) {
                                    cout << "index: cst_cn<csa_sada, lcp_sada, 8, 4>" << std::endl;
                                    out_file += ".cst_cn_sa_sa_8_4";
                                    create_index<cstds::cst_cn<csa_sada<>, lcp_support_sada<>,
                                                           cstds::npr_support_cnr, 8, 4> >(file, tmp_dir, out_file);
                                }
                                else if (s == 8) {
                                    cout << "index: cst_cn<csa_sada, lcp_sada, 8, 8>" << std::endl;
                                    out_file += ".cst_cn_sa_sa_8_8";
                                    create_index<cstds::cst_cn<csa_sada<>, lcp_support_sada<>,
                                                           cstds::npr_support_cnr, 8, 8> >(file, tmp_dir, out_file);
                                }
                                else
                                    cout << "Error: the -s option must be 4, 8" << endl;
                                break;
                            case 16:
                                if (s == 4) {
                                    cout << "index: cst_cn<csa_sada, lcp_sada, 16, 4>" << std::endl;
                                    out_file += ".cst_cn_sa_sa_16_4";
                                    create_index<cstds::cst_cn<csa_sada<>, lcp_support_sada<>,
                                                           cstds::npr_support_cnr, 16, 4> >(file, tmp_dir, out_file);
                                }
                                else if (s == 8) {
                                    cout << "index: cst_cn<csa_sada, lcp_sada, 16, 8>" << std::endl;
                                    out_file += ".cst_cn_sa_sa_16_8";
                                    create_index<cstds::cst_cn<csa_sada<>, lcp_support_sada<>,
                                                           cstds::npr_support_cnr, 16, 8> >(file, tmp_dir, out_file);
                                }
                                else
                                    cout << "Error: the -s option must be 4, 8" << endl;
                                break;
                            case 32:
                                if (s == 4) {
                                    cout << "index: cst_cn<csa_sada, lcp_sada, 32, 4>" << std::endl;
                                    out_file += ".cst_cn_sa_sa_32_4";
                                    create_index<cstds::cst_cn<csa_sada<>, lcp_support_sada<>,
                                                           cstds::npr_support_cnr, 32, 4> >(file, tmp_dir, out_file);
                                }
                                else if (s == 8) {
                                    cout << "index: cst_cn<csa_sada, lcp_sada, 32, 8>" << std::endl;
                                    out_file += ".cst_cn_sa_sa_32_8";
                                    create_index<cstds::cst_cn<csa_sada<>, lcp_support_sada<>,
                                                           cstds::npr_support_cnr, 32, 8> >(file, tmp_dir, out_file);
                                }
                                else
                                    cout << "Error: the -s option must be 4, 8" << endl;
                                break;
                            default:
                                cout << "Error: the -b option must be 8, 16, or 32" << endl;
                        }
                    }
                    else
                        cout << "Error: the -l option must be in [0,1]" << endl;

                    break;
                default:
                    cout << "Error: the -c option must be in [0,1]" << endl;
            }
            break;
        case 2:  // CST_SADA
            switch (c) {
                case 0:
                    if (l == 0) {
                        cout << "index: cst_sada<csa_wt, lcp_dac>" << std::endl;
                        out_file += ".cst_sada_wt_dac";
                        create_index<sdsl::cst_sada<csa_wt<>, lcp_dac<>> >(file, tmp_dir, out_file);
                    }
                    else if (l == 1) {
                        cout << "index: cst_sada<csa_wt, lcp_support_tree2>" << std::endl;
                        out_file += ".cst_sada_wt_t2";
                        create_index<sdsl::cst_sada<csa_wt<>, lcp_support_tree2<>> >(file, tmp_dir, out_file);
                    }
                    else
                        cout << "Error: the -l option must be in [0,1]" << endl;
                    break;
                case 1:
                    if (l == 0) {
                        cout << "index: cst_sada<csa_sada, lcp_dac>" << std::endl;
                        out_file += ".cst_sada_sa_dac";
                        create_index<sdsl::cst_sada<csa_sada<>, lcp_dac<> > >(file, tmp_dir, out_file);
                    }
                    else if (l == 1) {
                        cout << "index: cst_sada<csa_sada, lcp_support_tree2>" << std::endl;
                        out_file += ".cst_sada_sa_t2";
                        create_index<sdsl::cst_sada<csa_sada<>, lcp_support_tree2<>> >(file, tmp_dir, out_file);
                    }
                    else
                       cout << "Error: the -l option must be in [0,1]" << endl;
                    break;
                default:
                    cout << "Error: the -c option must be in [0,1]" << endl;
            }
            break;
        case 3: //CST_SCT3
            switch (c) {
                case 0:
                    if (l == 0) {
                        cout << "index: cst_sct3<csa_wt, lcp_dac>" << std::endl;
                        out_file += ".cst_sct3_wt_dac";
                        create_index<sdsl::cst_sct3<csa_wt<>, lcp_dac<>> >(file, tmp_dir, out_file); }
                    else if (l == 1) {
                        cout << "index: cst_sct3<csa_wt, lcp_support_tree2>" << std::endl;
                        out_file += ".cst_sct3_wt_t2";
                        create_index<sdsl::cst_sct3<csa_wt<>, lcp_support_tree2<>> >(file, tmp_dir, out_file);
                    }
                    else
                        cout << "Error: the -l option must be in [0,1]" << endl;
                    break;
                case 1:
                    if (l == 0) {
                        cout << "index: cst_sct3<csa_sada, lcp_dac>" << std::endl;
                        out_file += ".cst_sct3_sa_dac";
                        create_index<sdsl::cst_sct3<csa_sada<>, lcp_dac<> > >(file, tmp_dir, out_file);
                    }
                    else if (l == 1){
                        cout << "index: cst_sct3<csa_sada, lcp_support_tree2>" << std::endl;
                        out_file += ".cst_sct3_sa_t2";
                        create_index<sdsl::cst_sct3<csa_sada<>, lcp_support_tree2<>> >(file, tmp_dir, out_file);
                    }
                    else
                        cout << "Error: the -l option must be in [0,1]" << endl;
                    break;
                default:
                    cout << "Error: the -c option must be in [0,1]" << endl;
            }
            break;
        default:
            cout << "index_type must be a value in [0,3]" << endl;
    }

    return 0;
}
