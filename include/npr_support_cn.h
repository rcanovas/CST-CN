/* cstds - compressed suffix tree data structure
Copyright (C)2016-2017 Rodrigo Canovas
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see http://www.gnu.org/licenses/ .
*/

#ifndef CCST_CN_NPR_CN_SUPPORT_H
#define CCST_CN_NPR_CN_SUPPORT_H

#include "npr.h"
#include <sdsl/int_vector.hpp>
#include <vector>

using namespace sdsl;

namespace cstds {

    //! A class to represent the NPR operations over a LCP array in compressed form.
    /*!
    * \tparam t_lcp     Type of the Underlying LCP.
    * \tparam bloc_size Block size used got the npr_cn tree
    * \par Reference
    *   Rodrigo Canovas  and Gonzalo Navarro
    *   Practical compressed suffix trees
    *   SEA 2010: 94-105
    */
    template<class t_lcp = sdsl::lcp_dac<>, uint8_t block_size = 32, uint8_t sbs = 8>
    class _npr_support_cn {

    public:
        typedef sdsl::int_vector<>::size_type size_type;
        typedef t_lcp lcp_type;

        // inner class which is used in CSTs to parametrize npr classes
        // with information about the CST.
        template<class Cst>
        struct type {
            typedef _npr_support_cn npr_type;
        };

    private:
        const lcp_type *m_lcp;
        std::vector<sdsl::int_vector<> > min_array; //array for each level that contain the min value of each block
        std::vector<sdsl::int_vector<> > pos_array; //array for each level that contain the local position of the min value of each block


        void
        copy(const _npr_support_cn &npr_c) {
            m_lcp = npr_c.m_lcp;
            min_array = npr_c.min_array;
            pos_array = npr_c.pos_array;
        }

    public:

        //! Default Constructor
        _npr_support_cn() {}

        //! Copy constructor
        _npr_support_cn(const _npr_support_cn &npr_c) {
            copy(npr_c);
        }

        //! Move constructor
        _npr_support_cn(_npr_support_cn &&npr_c) {
            *this = std::move(npr_c);
        }

        //! Constructor. Note that sbs is not used for this version
        _npr_support_cn(const t_lcp *f_lcp) {
            set_lcp(f_lcp); //assign lcp to be used
            //construct data
            size_type tmp_min, min, tmp_pos, tmp_start, b_size = block_size;
            size_type n = m_lcp->size();
            size_type n_levels = (size_type) (ceil(log(n) / log(block_size)));
            size_type level_size = (size_type) ((n + block_size - 1) / block_size);
            size_type last_level_size = n;
            size_type bits_pos = bits::hi(block_size) + 1;
            size_type bits_min = bits::hi(n) + 1;
            min_array.resize(n_levels);
            pos_array.resize(n_levels);
            //create first level
            min_array[0] = int_vector<>(level_size, 0, bits_min);
            pos_array[0] = int_vector<>(level_size, 0, bits_pos);
            for (size_type i = 0; i < level_size; ++i) {
                min = n;
                tmp_start = i * block_size;
                if (tmp_start + block_size >= last_level_size)
                    b_size = last_level_size - tmp_start;
                for (size_type j = 0; j < b_size; j++) {
                    tmp_min = (*m_lcp)[tmp_start + j];
                    if (tmp_min < min) {
                        min = tmp_min;
                        tmp_pos = j;
                    }
                }
                min_array[0][i] = min;
                pos_array[0][i] = tmp_pos;
            }
            //create the other levels
            for (size_type r = 1; r < n_levels; ++r) {
                last_level_size = level_size;
                level_size = (level_size + block_size - 1) / block_size;
                min_array[r] = int_vector<>(level_size, 0, bits_min);
                pos_array[r] = int_vector<>(level_size, 0, bits_pos);
                b_size = block_size;
                for (size_type i = 0; i < level_size; ++i) {
                    min = n;
                    tmp_start = i * block_size;
                    if (tmp_start + block_size >= last_level_size)
                        b_size = last_level_size - tmp_start;
                    for (size_type j = 0; j < b_size; j++) {
                        tmp_min = min_array[r - 1][tmp_start + j];
                        if (tmp_min < min) {
                            min = tmp_min;
                            tmp_pos = j;
                        }
                    }
                    min_array[r][i] = min;
                    pos_array[r][i] = tmp_pos;
                }
            }
        }

        void
        set_lcp(const t_lcp *f_lcp) {
            m_lcp = f_lcp;
        }

        //! Returns if the data structure is empty.
        bool
        empty() const {
            return m_lcp->empty();
        }

        //! Swap method for _npr_support_cn
        void
        swap(_npr_support_cn &npr_c) {
            min_array.swap(npr_c.min_array);
            pos_array.swap(npr_c.pos_array);
        }

        //! Assignment Operator.
        _npr_support_cn &operator=(const _npr_support_cn &npr_c) {
            if (this != &npr_c) {
                copy(npr_c);
            }
            return *this;
        }

        //! Assignment Move Operator.
        _npr_support_cn &operator=(_npr_support_cn &&npr_c) {
            if (this != &npr_c) {
                m_lcp = std::move(npr_c.m_lcp);
                min_array = std::move(npr_c.min_array);
                pos_array = std::move(npr_c.pos_array);
            }
            return *this;
        }

        //! Serialize to a stream.
        size_type
        serialize(std::ostream &out, structure_tree_node *v = nullptr, std::string name = "") const {
            structure_tree_node *child = structure_tree::add_child(v, name, sdsl::util::class_name(*this));
            size_type levels = min_array.size();
            size_type written_bytes = 0;
            written_bytes += write_member(levels, out, child, "levels");
            for (size_type i = 0; i < levels; ++i) {
                written_bytes += min_array[i].serialize(out, child, "min level");
                written_bytes += pos_array[i].serialize(out, child, "pos level");
            }
            structure_tree::add_size(child, written_bytes);
            //std::cout << "NPR uses: " << (written_bytes * 8.0) / m_lcp->size() << "n bits" << std::endl;
            return written_bytes;
        }

        //! Load from a stream.
        void
        load(std::istream &in, const t_lcp *llcp = nullptr) {
            size_type levels = 0;
            m_lcp = llcp;
            read_member(levels, in);
            min_array.resize(levels);
            pos_array.resize(levels);
            for (size_type i = 0; i < levels; ++i) {
                min_array[i].load(in);
                pos_array[i].load(in);
            }
        }

//FUNCTIONS
        //! Get the position of the next smaller value than LCP[i] within [i+1,n]
        //! Also returns the value found in l_value.
        size_type
        nsv(size_type i, size_type &l_value) const {
            size_type value_v = (*m_lcp)[i];
            return fwd_nsv(i + 1,  value_v, l_value);
        }


        //! Get the position of the next smaller value than d starting from i.
        size_type
        fwd_nsv(size_type i,  size_type d, size_type &l_value) const {
            size_type block, until = 0, value, n = m_lcp->size();
            l_value = n; //not found yet
            if (d == 0 or i > n - 1)
                return n;
            block = i / block_size;
            if (min_array[0][block] < d) { //need to search in the first block
                until = block_size * (block + 1);
                if (until > n)
                    until = n;
                for (size_type j = i; j < until; ++j) {
                    value = (*m_lcp)[j];
                    if (value < d) {
                        l_value = value;
                        return j;
                    }
                }
            }
            if (until == n)  //last block
                return n;
            block = find_nsv_block(d, block + 1, 0, l_value); //need to find block containing a value smaller than d
            if (block == min_array[0].size())
                return n;
            until = block * block_size + pos_array[0][block];
            if (l_value == d - 1) //we already found the position of nsv
                return until;
            for (size_type j = block * block_size; j < until; ++j) { //look if there is a small value earlier
                value = (*m_lcp)[j];
                if (value < d) {
                    l_value = value;
                    return j;
                }
            }
            return until; // it was the last one
        }

        //! Get the position of the previous smaller value than LCP[i] within [0,i-1]
        //! Also returns the value found in l_value.
        size_type
        psv(size_type i, size_type &l_value) const {
            size_type value = m_lcp->size();
            if (i == 0) {
                l_value = value;
                return value;
            }
            value = (*m_lcp)[i];
            return bwd_psv(i - 1,  value, l_value);
        }

        //! Get the position of the previous smaller value than d starting from i.
        size_type
        bwd_psv(size_type i,  size_type d, size_type &l_value) const {
            size_type block, value, n;
            size_type until = m_lcp->size();
            l_value = n = until; // not found yet
            if (d == 0)
                return n;
            block = i / block_size;
            if (min_array[0][block] < d) { //need to search in the first block
                until = block_size * block;
                for (size_type j = i; j >= until; --j) {
                    value = (*m_lcp)[j];
                    if (value < d) {
                        l_value = value;
                        return j;
                    }
                }
            }
            if (until == 0)
                return n;
            block = find_psv_block(d, block - 1, 0, l_value); //need to find block containing a value smaller than d
            if (block == min_array[0].size())
                return n;
            until = block * block_size + pos_array[0][block];
            for (size_type j = (block + 1) * block_size - 1; j > until; --j) { //look if there is a small value earlier
                value = (*m_lcp)[j];
                if (value < d) {
                    l_value = value;
                    return j;
                }
            }
            return until; // the smaller value was the one pointed
        }

        //! Get the left most position of the minimum value in the interval [i,j].
        //! We assumed that always 0 <= i <= j < n.
        size_type
        rmq(size_type i, size_type j, size_type &l_value) const {
            size_type r_block, l_block, value, until, min_bpos, block;
            size_type min_rmq = m_lcp->size(), min_pos, aux_rmq;
            l_block = i / block_size;
            r_block = j / block_size;
            //compute left block first
            until = (l_block + 1) * block_size - 1;
            if (until > j)
                until = j;
            min_bpos = l_block * block_size + pos_array[0][l_block]; //position of the min in the block
            if (min_bpos >= i and min_bpos <= until) {
                min_rmq = min_array[0][l_block];
                min_pos = min_bpos;
            }
            else {
                for (size_type r = i; r <= until; ++r) {
                    value = (*m_lcp)[r];
                    if (value < min_rmq) {
                        min_rmq = value;
                        min_pos = r;
                        if (min_rmq == 0)
                            break;
                    }
                }
            }
            if (until == j or min_rmq == 0) {  //case left_block == right_block and if we find a 0
                l_value = min_rmq;
                return min_pos;
            }
            l_block++;
            //compute middle blocks
            if (l_block < r_block) {
                block = find_rmq_block(l_block, r_block - 1, aux_rmq, 0);
                if (aux_rmq < min_rmq) {
                    min_rmq = aux_rmq;
                    min_pos = block * block_size + pos_array[0][block];
                    if (min_rmq == 0) {
                        l_value = min_rmq;
                        return min_pos;
                    }
                }
            }
            //compute min right block
            until = j;
            min_bpos = r_block * block_size + pos_array[0][r_block];
            if (min_array[0][r_block] < min_rmq) {
                if (min_bpos <= until) {
                    l_value = min_array[0][r_block];
                    return min_bpos;
                } else {
                    for (size_type r = r_block * block_size; r <= until; ++r) {
                        value = (*m_lcp)[r];
                        if (value < min_rmq) {
                            min_rmq = value;
                            min_pos = r;
                            if (min_rmq == 0)
                                break;
                        }
                    }
                }
            }
            l_value = min_rmq;
            return min_pos;
        }

    private:

        //! Find the first block in the min_array[level] starting from b such
        //! that contain a value smaller than d.
        size_type
        find_nsv_block(size_type  d, size_type b, size_type level, size_type &l_value) const {
            size_type block, until = 0, value, n = min_array[level].size();
            block = b / block_size; //local block
            if (min_array[level + 1][block] < d) { //need to search in the first block
                until = block_size * (block + 1);
                if (until > n)
                    until = n;
                for (size_type j = b; j < until; ++j) {
                    value = min_array[level][j];
                    if (value < d) {
                        l_value = value;
                        return j;
                    }
                }
            }
            if (until == n)
                return n;
            //search following blocks
            block = find_nsv_block(d, block + 1, level + 1, l_value);
            if (block == min_array[level + 1].size())
                return n;
            until = block * block_size + pos_array[level + 1][block];
            if (l_value == d - 1) //we already found the position of nsv
                return until;
            for (size_type j = block * block_size; j < until; ++j) {
                value = min_array[level][j];
                if (value < d) {
                    l_value = value;
                    return j;
                }
            }
            return until; // the smaller value was the one pointed
        }

        //! Find the last block in the min_array[level] between [0,b] such
        //! that contain a value smaller than d.
        size_type
        find_psv_block(size_type  d, size_type b, size_type level, size_type &l_value) const {
            size_type block, until = min_array[level].size(), value, n = min_array[level].size();
            block = b / block_size;
            if (min_array[level + 1][block] < d) { //need to search in the first block
                until = block_size * block;
                for (size_type j = b; j >= until; --j) {
                    value = min_array[level][j];
                    if (value < d) {
                        l_value = value;
                        return j;
                    }
                }
            }
            if (until == 0)
                return n;
            block = find_psv_block(d, block - 1, level + 1, l_value); //search the other level
            if (block == min_array[level + 1].size())
                return n;
            until = block * block_size + pos_array[level + 1][block];
            for (size_type j = (block + 1) * block_size - 1; j > until; --j) {
                value = min_array[level][j];
                if (value < d) {
                    l_value = value;
                    return j;
                }
            }
            return until; // the smaller value was the one pointed
        }


        //! Find the minimum value between the blocks i and j at "level"
        size_type
        find_rmq_block(size_type i, size_type j, size_type &l_value, size_type level) const {
            size_type r_block, l_block, value, until, min_bpos, block;
            size_type min_rmq = m_lcp->size(), min_pos, aux_rmq;
            l_block = i / block_size;
            r_block = j / block_size;
            //compute left part first
            until = (l_block + 1) * block_size - 1;
            if (until > j)
                until = j;
            min_bpos = l_block * block_size + pos_array[level + 1][l_block];
            if (min_bpos >= i and min_bpos <= until) {
                min_rmq = min_array[level + 1][l_block];
                min_pos = min_bpos;
            } else {
                for (size_type r = i; r <= until; ++r) {
                    value = min_array[level][r];
                    if (value < min_rmq) {
                        min_rmq = value;
                        min_pos = r;
                        if (min_rmq == 0)
                            break;
                    }
                }
            }
            if (until == j or min_rmq == 0) {  //case l_block == r_block and if we find a 0
                l_value = min_rmq;
                return min_pos;
            }
            l_block++;
            if (l_block < r_block) { //compute middle section
                block = find_rmq_block(l_block, r_block - 1, aux_rmq, level + 1);
                if (aux_rmq < min_rmq) {
                    min_rmq = aux_rmq;
                    min_pos = block * block_size + pos_array[level + 1][block];
                    if (min_rmq == 0) {
                        l_value = min_rmq;
                        return min_pos;
                    }
                }
            }
            //compute min right block
            until = j;
            min_bpos = r_block * block_size + pos_array[level + 1][r_block];
            if (min_array[level + 1][r_block] < min_rmq) {
                if (min_bpos <= until) {
                    l_value = min_array[level + 1][r_block];
                    return min_bpos;
                } else {
                    for (size_type r = r_block * block_size; r <= until; ++r) {
                        value = min_array[level][r];
                        if (value < min_rmq) {
                            min_rmq = value;
                            min_pos = r;
                            if (min_rmq == 0)
                                break;
                        }
                    }
                }
            }
            l_value = min_rmq;
            return min_pos;
        }

    }; //end class

    //! Helper class which provides _npr_support_cn the context of a LCP.
    struct npr_support_cn {
        template<class t_cst, uint8_t block_size, uint8_t sbs>
        using type = _npr_support_cn<typename t_cst::lcp_type, block_size, sbs>;
    };

}

#endif //CCST_CN_NPR_CN_SUPPORT_H
