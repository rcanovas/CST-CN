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

#include <random>
#include <iostream>
#include <sdsl/suffix_trees.hpp>
#include "./../include/cst_cn.h"

using namespace std;

//! Takes x random leaves of the CST. For each random leaf
//	v we add all nodes of the path from v to the root to the sample
//	Operations measured: parent(v), depth(v), first child(v),
//                       sibling(v), node_depth(v),
//                       and child(v, c) (chosen characters c from
//                       random positions in T).
// Based on the experiments done by Simon Gog in his PhD thesis
template<class idx_type>
void
test_with_sample_v1(idx_type cst) {
    typedef typename idx_type::node_type node_type;
    typedef typename idx_type::char_type char_type;
    vector<node_type> sample;
    vector<char_type> symbol;

    //std::random_device rd;
    //std::mt19937 mt(rd());

    std::default_random_engine generator;
    std::uniform_real_distribution<double> dist(1.0, 1.0 * cst.csa.size());
    std::default_random_engine generator2;
    std::uniform_real_distribution<double> let(0.0, 1.0 * cst.csa.size() - 1);
    uint64_t pos = 0, let_pos = 0;
    auto root = cst.root();
    for (uint64_t i = 0; i < 10000; ++ i) {
        //pos = (uint64_t)dist(mt);
        //if you want to always generate the same random numbers whenever the test is run
        pos = (uint64_t)dist(generator);
        auto node = cst.select_leaf(pos);
        node = cst.parent(node);
        while (node !=  root){
            sample.push_back(node);
            let_pos = (uint64_t)let(generator2);
            symbol.push_back(sdsl::first_row_symbol(let_pos, cst.csa));
            node = cst.parent(node);
        }
    }
    pos = sample.size();
    cout << "Sample V1 size: " << pos << endl;

    cout << "Parent: ";
    using timer = std::chrono::high_resolution_clock;
    auto start = timer::now();
    for (uint64_t i = 0; i < pos; ++ i)
        auto p = cst.parent(sample[i]);
    auto stop = timer::now();
    auto elapsed = stop - start;
    cout << ((chrono::duration_cast<chrono::nanoseconds>(elapsed).count() * 1.0)) / pos << " nanosec" << endl;

    cout << "Depth: ";
    start = timer::now();
    for (uint64_t i = 0; i < pos; ++ i)
        auto p = cst.depth(sample[i]);
    stop = timer::now();
    elapsed = stop - start;
    cout << ((chrono::duration_cast<chrono::nanoseconds>(elapsed).count() * 1.0)) / pos << " nanosec" << endl;

    cout << "First Child: ";
    start = timer::now();
    for (uint64_t i = 0; i < pos; ++ i)
        auto p = cst.select_child(sample[i], 1);
    stop = timer::now();
    elapsed = stop - start;
    cout << ((chrono::duration_cast<chrono::nanoseconds>(elapsed).count() * 1.0)) / pos << " nanosec" << endl;

    cout << "Sibling: ";
    start = timer::now();
    for (uint64_t i = 0; i < pos; ++ i)
        auto p = cst.sibling(sample[i]);
    stop = timer::now();
    elapsed = stop - start;
    cout << ((chrono::duration_cast<chrono::nanoseconds>(elapsed).count() * 1.0)) / pos << " nanosec" << endl;

    cout << "Node-Depth: ";
    start = timer::now();
    for (uint64_t i = 0; i < pos; ++ i)
        auto p = cst.node_depth(sample[i]);
    stop = timer::now();
    elapsed = stop - start;
    cout << ((chrono::duration_cast<chrono::nanoseconds>(elapsed).count() * 1.0)) / pos << " nanosec" << endl;

    cout << "Child: ";
    start = timer::now();
    for (uint64_t i = 0; i < pos; ++ i) {
        auto p = cst.child(sample[i], symbol[i]);
        //cout << cst.depth(p) << endl;
    }
    stop = timer::now();
    elapsed = stop - start;
    cout << ((chrono::duration_cast<chrono::nanoseconds>(elapsed).count() * 1.0)) / pos << " nanosec" << endl;
}


//! Takes x random leaves of the CST. For each parent v of a
// 	random leaf we call the suffix link operation until we
//  reach the root and add all these nodes to sample.
//  Operations measured: sl(v).
template<class idx_type>
void
test_with_get_sample_v2(idx_type cst) {
    typedef typename idx_type::node_type node_type;
    vector<node_type> sample;
    //std::random_device rd;
    //std::mt19937 mt(rd());
    std::default_random_engine generator;
    std::uniform_real_distribution<double> dist(1.0, 1.0 * cst.csa.size());
    uint64_t pos = 0;
    auto root = cst.root();
    for (uint64_t i = 0; i < 1000; ++ i) {
        //pos = (uint64_t)dist(mt);
        //if you want to always generate the same random numbers whenever the test is run
        pos = (uint64_t)dist(generator);
        auto node = cst.select_leaf(pos);
        node = cst.parent(node);
        while (node !=  root){
            sample.push_back(node);
            node = cst.sl(node);
        }
    }

    pos = sample.size();
    std::cout << "Sample V2 size: " << pos << std::endl;

    std::cout << "Slink: ";
    using timer = std::chrono::high_resolution_clock;
    auto start = timer::now();
    for (uint64_t i = 0; i < pos; ++ i)
        auto slink = cst.sl(sample[i]);
    auto stop = timer::now();
    auto elapsed = stop - start;
    cout << ((chrono::duration_cast<chrono::nanoseconds>(elapsed).count() * 1.0)) / pos << " nanosec" << endl;
}


//! Take x random leaf pairs.
//! Operation measured: lca(v, w).
template<class idx_type>
void
test_with_sample_v3(idx_type cst) {
    typedef typename idx_type::node_type node_type;
    vector<node_type> sample;
    //std::random_device rd;
    //std::mt19937 mt(rd());
    std::default_random_engine generator;
    std::uniform_real_distribution<double> dist(1.0, 1.0 * cst.csa.size());
    uint64_t pos1 = 0, pos2 = 0;
    node_type node1, node2;
    auto root = cst.root();
    for (uint64_t i = 0; i < 100000; ++ i) {
        //pos1 = (uint64_t)dist(mt);
        //pos2 = (uint64_t)dist(mt);
        //if you want to always generate the same random numbers whenever the test is run
        pos1 = (uint64_t)dist(generator);
        pos2 = (uint64_t)dist(generator);
        if (pos1 < pos2) {
            node1 = cst.select_leaf(pos1);
            node2 = cst.select_leaf(pos2);
        }
        else {
            node1 = cst.select_leaf(pos2);
            node2 = cst.select_leaf(pos1);
        }
        sample.push_back(node1);
        sample.push_back(node2);
    }

    std::cout << "Sample V3 size: " << sample.size() << std::endl;

    std::cout << "LCA: ";
    using timer = std::chrono::high_resolution_clock;
    auto start = timer::now();
    for (uint64_t i = 0; i < 200000; i += 2)
        auto lca = cst.lca(sample[i], sample[i + 1]);
    auto stop = timer::now();
    auto elapsed = stop - start;
    cout << ((chrono::duration_cast<chrono::nanoseconds>(elapsed).count() * 1.0)) / 100000.0 << " nanosec" << endl;
}


template<class idx_type>
void
test_cst(string file) {
    idx_type idx;
    std::ifstream f_in(file, std::ios::in | std::ios::binary);
    if(!f_in) {
        std::cerr << "Failed to open file " << file;
        exit(1);
    }
    idx.load(f_in);
    uint64_t size_idx = sdsl::size_in_bytes(idx);
    std::cout << "Size Text: " << idx.size() << std::endl;
    std::cout << "Size in bytes: " << size_idx << " bytes" << std::endl;
    std::cout << "Size in bits: " << (size_idx * 8.0 / idx.size()) << "n bits" << std::endl;

    test_with_sample_v1<idx_type>(idx);
    test_with_get_sample_v2<idx_type>(idx);
    test_with_sample_v3<idx_type>(idx);

}

int main(int argc, char* argv[]) {

    if(argc < 2) {
        cout << "Usage: " << argv[0] << " index_file <opt>" << endl;
        cout << "opt (index details needed): " << endl;
        cout << "-w Index_type. Default = 0" << endl;
        cout << "    ---+--------------------" << endl;
        cout << "     0 | CST_CN with NPR-CN" << endl;
        cout << "     1 | CST_CN with NPR-CNR" << endl;
        cout << "     2 | CST_SADA" << endl;
        cout << "     3 | CST_SCT3" << endl;
        cout << "-c suffix array: CSA used within the CST used. Default = 0 " << endl;
        cout << "    ---+--------------------" << endl;
        cout << "     0 | CSA_WT" << endl;
        cout << "     1 | CSA_SADA" << endl;
        cout << "-l lcp array: LCP used within the CST chosen. Default = 0 " << endl;
        cout << "    ---+--------------------" << endl;
        cout << "     0 | LCP_DAC" << endl;
        cout << "     1 | LCP_SUPPORT_SADA (for CST-CN) and LCP_SUPPORT_TREE2 (for CST-SCT3 and CST_SADA)" << endl;
        cout << "-b block_size: Block size for NPR of CN and CNR (values accepted in this test: 32, 16, 8). Default = 32 " << endl;
        cout << "-s small block_size: Small Block size for NPR of CNR (values accepted in this test: 8, 4). Default = 8 " << endl;
        return 1;
    }

    string file = argv[1];
    int w = 0, c = 0, l = 0, b = 32, s = 8;

    int o;
    while((o = getopt (argc, argv, "w:c:l:b:s:")) != -1){
        switch (o) {
            case 'w': w = atoi(optarg); break;
            case 'c': c = atoi(optarg); break;
            case 'l': l = atoi(optarg); break;
            case 'b': b = atoi(optarg); break;
            case 's': s = atoi(optarg); break;
            case '?':
                if(optopt == 'w' || optopt == 'c' || optopt == 'l' ||
                        optopt == 'b' ||  optopt == 's')
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
                                test_cst<cstds::cst_cn<csa_wt<>, lcp_dac<>,
                                        cstds::npr_support_cn, 8> >(file);
                                break;
                            case 16:
                                cout << "index: cst_cn<csa_wt, lcp_dac, 16>" << std::endl;
                                test_cst<cstds::cst_cn<csa_wt<>, lcp_dac<>,
                                        cstds::npr_support_cn, 16> >(file);
                                break;
                            case 32:
                                cout << "index: cst_cn<csa_wt, lcp_dac, 32>" << std::endl;
                                test_cst<cstds::cst_cn<csa_wt<>, lcp_dac<>,
                                        cstds::npr_support_cn, 32> >(file);
                                break;
                            default:
                                cout << "Error: the -b option must be 8, 16, or 32" << endl;
                        }
                    }
                    else if (l == 1) { //LCP-SUPPORT-SADA
                        switch (b) {
                            case 8:
                                cout << "index: cst_cn<csa_wt, lcp_support_sada, 8>" << std::endl;
                                test_cst<cstds::cst_cn<csa_wt<>, lcp_support_sada<>,
                                        cstds::npr_support_cn, 8> >(file);
                                break;
                            case 16:
                                cout << "index: cst_cn<csa_wt, lcp_support_sada, 16>" << std::endl;
                                test_cst<cstds::cst_cn<csa_wt<>, lcp_support_sada<>,
                                        cstds::npr_support_cn, 16> >(file);
                                break;
                            case 32:
                                cout << "index: cst_cn<csa_wt, lcp_support_sada, 32>" << std::endl;
                                test_cst<cstds::cst_cn<csa_wt<>, lcp_support_sada<>,
                                        cstds::npr_support_cn, 32> >(file);
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
                                test_cst<cstds::cst_cn<csa_sada<>, lcp_dac<>,
                                        cstds::npr_support_cn, 8> >(file);
                                break;
                            case 16:
                                cout << "index: cst_cn<csa_sada, lcp_dac, 16>" << std::endl;
                                test_cst<cstds::cst_cn<csa_sada<>, lcp_dac<>,
                                        cstds::npr_support_cn, 16> >(file);
                                break;
                            case 32:
                                cout << "index: cst_cn<csa_sada, lcp_dac, 32>" << std::endl;
                                test_cst<cstds::cst_cn<csa_sada<>, lcp_dac<>,
                                        cstds::npr_support_cn, 32> >(file);
                                break;
                            default:
                                cout << "Error: the -b option must be 8, 16, or 32" << endl;
                        }
                    }
                    else if (l == 1) { //LCP-SUPPORT-SADA
                        switch (b) {
                            case 8:
                                cout << "index: cst_cn<csa_sada, lcp_support_sada, 8>" << std::endl;
                                test_cst<cstds::cst_cn<csa_sada<>, lcp_support_sada<>,
                                        cstds::npr_support_cn, 8> >(file);
                                break;
                            case 16:
                                cout << "index: cst_cn<csa_sada, lcp_support_sada, 16>" << std::endl;
                                test_cst<cstds::cst_cn<csa_sada<sdsl::enc_vector<>, 32, 32>, lcp_support_sada<>,
                                        cstds::npr_support_cn, 16> >(file);
                                break;
                            case 32:
                                cout << "index: cst_cn<csa_sada, lcp_support_sada, 32>" << std::endl;
                                test_cst<cstds::cst_cn<csa_sada<sdsl::enc_vector<>, 32, 32>, lcp_support_sada<>,
                                        cstds::npr_support_cn, 32> >(file);
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
                                    test_cst<cstds::cst_cn<csa_wt<>, lcp_dac<>,
                                            cstds::npr_support_cnr, 8, 4> >(file);
                                }
                                else if (s == 8) {
                                    cout << "index: cst_cn<csa_wt, lcp_dac, 8, 8>" << std::endl;
                                    test_cst<cstds::cst_cn<csa_wt<>, lcp_dac<>,
                                            cstds::npr_support_cnr, 8, 8> >(file);
                                }
                                else
                                    cout << "Error: the -s option must be 4, 8" << endl;
                                break;
                            case 16:
                                if (s == 4) {
                                    cout << "index: cst_cn<csa_wt, lcp_dac, 16, 4>" << std::endl;
                                    test_cst<cstds::cst_cn<csa_wt<>, lcp_dac<>,
                                            cstds::npr_support_cnr, 16, 4> >(file);
                                }
                                else if (s == 8) {
                                    cout << "index: cst_cn<csa_wt, lcp_dac, 16, 8>" << std::endl;
                                    test_cst<cstds::cst_cn<csa_wt<>, lcp_dac<>,
                                            cstds::npr_support_cnr, 16, 8> >(file);
                                }
                                else
                                    cout << "Error: the -s option must be 4, 8" << endl;
                                break;
                            case 32:
                                if (s == 4) {
                                    cout << "index: cst_cn<csa_wt, lcp_dac, 32, 4>" << std::endl;
                                    test_cst<cstds::cst_cn<csa_wt<>, lcp_dac<>,
                                            cstds::npr_support_cnr, 32, 4> >(file);
                                }
                                else if (s == 8) {
                                    cout << "index: cst_cn<csa_wt, lcp_dac, 32, 8>" << std::endl;
                                    test_cst<cstds::cst_cn<csa_wt<>, lcp_dac<>,
                                            cstds::npr_support_cnr, 32, 8> >(file);
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
                                    test_cst<cstds::cst_cn<csa_wt<>, lcp_support_sada<>,
                                            cstds::npr_support_cnr, 8, 4> >(file);
                                }
                                else if (s == 8) {
                                    cout << "index: cst_cn<csa_wt, lcp_sada, 8, 8>" << std::endl;
                                    test_cst<cstds::cst_cn<csa_wt<>, lcp_support_sada<>,
                                            cstds::npr_support_cnr, 8, 8> >(file);
                                }
                                else
                                    cout << "Error: the -s option must be 4, 8" << endl;
                                break;
                            case 16:
                                if (s == 4) {
                                    cout << "index: cst_cn<csa_wt, lcp_sada, 16, 4>" << std::endl;
                                    test_cst<cstds::cst_cn<csa_wt<>, lcp_support_sada<>,
                                            cstds::npr_support_cnr, 16, 4> >(file);
                                }
                                else if (s == 8) {
                                    cout << "index: cst_cn<csa_wt, lcp_sada, 16, 8>" << std::endl;
                                    test_cst<cstds::cst_cn<csa_wt<>, lcp_support_sada<>,
                                            cstds::npr_support_cnr, 16, 8> >(file);
                                }
                                else
                                    cout << "Error: the -s option must be 4, 8" << endl;
                                break;
                            case 32:
                                if (s == 4) {
                                    cout << "index: cst_cn<csa_wt, lcp_sada, 32, 4>" << std::endl;
                                    test_cst<cstds::cst_cn<csa_wt<>, lcp_support_sada<>,
                                            cstds::npr_support_cnr, 32, 4> >(file);
                                }
                                else if (s == 8) {
                                    cout << "index: cst_cn<csa_wt, lcp_sada, 32, 8>" << std::endl;
                                    test_cst<cstds::cst_cn<csa_wt<>, lcp_support_sada<>,
                                            cstds::npr_support_cnr, 32, 8> >(file);
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
                                    test_cst<cstds::cst_cn<csa_sada<>, lcp_dac<>,
                                            cstds::npr_support_cnr, 8, 4> >(file);
                                }
                                else if (s == 8) {
                                    cout << "index: cst_cn<csa_sada, lcp_dac, 8, 8>" << std::endl;
                                    test_cst<cstds::cst_cn<csa_sada<>, lcp_dac<>,
                                            cstds::npr_support_cnr, 8, 8> >(file);
                                }
                                else
                                    cout << "Error: the -s option must be 4, 8" << endl;
                                break;
                            case 16:
                                if (s == 4) {
                                    cout << "index: cst_cn<csa_sada, lcp_dac, 16, 4>" << std::endl;
                                    test_cst<cstds::cst_cn<csa_sada<>, lcp_dac<>,
                                            cstds::npr_support_cnr, 16, 4> >(file);
                                }
                                else if (s == 8) {
                                    cout << "index: cst_cn<csa_sada, lcp_dac, 16, 8>" << std::endl;
                                    test_cst<cstds::cst_cn<csa_sada<>, lcp_dac<>,
                                            cstds::npr_support_cnr, 16, 8> >(file);
                                }
                                else
                                    cout << "Error: the -s option must be 4, 8" << endl;
                                break;
                            case 32:
                                if (s == 4) {
                                    cout << "index: cst_cn<csa_sada, lcp_dac, 32, 4>" << std::endl;
                                    test_cst<cstds::cst_cn<csa_sada<>, lcp_dac<>,
                                            cstds::npr_support_cnr, 32, 4> >(file);
                                }
                                else if (s == 8) {
                                    cout << "index: cst_cn<csa_sada, lcp_dac, 32, 8>" << std::endl;
                                    test_cst<cstds::cst_cn<csa_sada<>, lcp_dac<>,
                                            cstds::npr_support_cnr, 32, 8> >(file);
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
                                    test_cst<cstds::cst_cn<csa_sada<>, lcp_support_sada<>,
                                            cstds::npr_support_cnr, 8, 4> >(file);
                                }
                                else if (s == 8) {
                                    cout << "index: cst_cn<csa_sada, lcp_sada, 8, 8>" << std::endl;
                                    test_cst<cstds::cst_cn<csa_sada<>, lcp_support_sada<>,
                                            cstds::npr_support_cnr, 8, 8> >(file);
                                }
                                else
                                    cout << "Error: the -s option must be 4, 8" << endl;
                                break;
                            case 16:
                                if (s == 4) {
                                    cout << "index: cst_cn<csa_sada, lcp_sada, 16, 4>" << std::endl;
                                    test_cst<cstds::cst_cn<csa_sada<>, lcp_support_sada<>,
                                            cstds::npr_support_cnr, 16, 4> >(file);
                                }
                                else if (s == 8) {
                                    cout << "index: cst_cn<csa_sada, lcp_sada, 16, 8>" << std::endl;
                                    test_cst<cstds::cst_cn<csa_sada<>, lcp_support_sada<>,
                                             cstds::npr_support_cnr, 16, 8> >(file);
                                }
                                else
                                    cout << "Error: the -s option must be 4, 8" << endl;
                                break;
                            case 32:
                                if (s == 4) {
                                    cout << "index: cst_cn<csa_sada, lcp_sada, 32, 4>" << std::endl;
                                    test_cst<cstds::cst_cn<csa_sada<>, lcp_support_sada<>,
                                            cstds::npr_support_cnr, 32, 4> >(file);
                                }
                                else if (s == 8) {
                                    cout << "index: cst_cn<csa_sada, lcp_sada, 32, 8>" << std::endl;
                                    test_cst<cstds::cst_cn<csa_sada<>, lcp_support_sada<>,
                                            cstds::npr_support_cnr, 32, 8> >(file);
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
                        test_cst<sdsl::cst_sada<csa_wt<>, lcp_dac<>> >(file);
                    }
                    else if (l == 1) {
                        cout << "index: cst_sada<csa_wt, lcp_support_tree2>" << std::endl;
                        test_cst<sdsl::cst_sada<csa_wt<>, lcp_support_tree2<>> >(file);
                    }
                    else
                        cout << "Error: the -l option must be in [0,1]" << endl;
                    break;
                case 1:
                    if (l == 0) {
                        cout << "index: cst_sada<csa_sada, lcp_dac>" << std::endl;
                        test_cst<sdsl::cst_sada<csa_sada<>, lcp_dac<> > >(file);
                    }
                    else if (l == 1) {
                        cout << "index: cst_sada<csa_sada, lcp_support_tree2>" << std::endl;
                        test_cst<sdsl::cst_sada<csa_sada<>, lcp_support_tree2<>> >(file);
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
                        test_cst<sdsl::cst_sct3<csa_wt<>, lcp_dac<>> >(file); }
                    else if (l == 1) {
                        cout << "index: cst_sct3<csa_wt, lcp_support_tree2>" << std::endl;
                        test_cst<sdsl::cst_sct3<csa_wt<>, lcp_support_tree2<>> >(file);
                    }
                    else
                        cout << "Error: the -l option must be in [0,1]" << endl;
                    break;
                case 1:
                    if (l == 0) {
                        cout << "index: cst_sct3<csa_sada, lcp_dac>" << std::endl;
                        test_cst<sdsl::cst_sct3<csa_sada<>, lcp_dac<> > >(file);
                    }
                    else if (l == 1){
                        cout << "index: cst_sct3<csa_sada, lcp_support_tree2>" << std::endl;
                        test_cst<sdsl::cst_sct3<csa_sada<>, lcp_support_tree2<>> >(file);
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
