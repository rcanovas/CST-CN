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

/*! \file cst_cn.hpp
    \brief cst_cn.hpp contains an implementation of Canovas and Navarro Compressed Suffix Tree.
    \author Rodrigo Canovas
*/

#ifndef CST_CN_HPP
#define CST_CN_HPP

#include <sdsl/int_vector.hpp>
#include <sdsl/lcp.hpp>
#include <sdsl/csa_wt.hpp> // for std initialization of cst_sct3
#include <sdsl/cst_iterators.hpp>
#include <sdsl/util.hpp>
#include <sdsl/sdsl_concepts.hpp>
#include <sdsl/construct.hpp>
#include <sdsl/suffix_tree_helper.hpp>
#include <sdsl/suffix_tree_algorithm.hpp>

#include "./npr.h"


using namespace sdsl;

namespace cstds {  //compressed suffix tree data structure


    //! A class for the Compressed Suffix Tree (CST-CN) proposed by Canovas and Navarro
    /*!
    * \tparam t_csa        Type of a CSA (member of this type is accessible via
    *                      member `csa`, default class is sdsl::wt).
    * \tparam t_lcp        Type of a LCP structure (member is accessible via member
    *                      `lcp`, default class is sdsl::lcp_dac).
    * \tparam t_npr        Type of NSV/PSV/RMQ support data structure used (default
    *                      class is cdsds::npr_support_cn<t_lcp>).
    * \par Reference
    *  Canovas, Rodrigo and Navarro, Gonzalo:
    *  Practical Compressed Suffix Trees.
    *  SEA 2010, 94-105
    *
    * @ingroup cst
    */
    template<class t_csa = csa_wt<>,
            class t_lcp = lcp_dac<>,
            class t_npr = npr_support_cn,
            uint8_t bs = 32,
            uint8_t sbs = 8>
    class cst_cn {

    public:
        typedef cst_dfs_const_forward_iterator<cst_cn>              const_iterator;
        typedef typename t_csa::size_type                           size_type;
        typedef t_csa                                               csa_type;
        typedef typename t_lcp::template type<cst_cn>               lcp_type;
        typedef typename t_npr::template type<cst_cn, bs, sbs>      npr_type;
        typedef typename t_csa::char_type                           char_type;
        typedef std::pair<size_type, size_type>                     node_type; // Nodes are represented by their interval over the CSA
        typedef size_type                                           leaf_type; // Index of a leaf

        typedef typename t_csa::alphabet_category                   alphabet_category;
        typedef typename t_csa::alphabet_type::comp_char_type       comp_char_type;

        typedef cst_tag                                             index_category;

    private:
        csa_type m_csa;
        lcp_type m_lcp;
        npr_type m_npr;

    public:
        const csa_type& csa = m_csa;
        const lcp_type& lcp = m_lcp;
        const npr_type& npr = m_npr;

        //! Default constructor
        cst_cn() {}

        //! Copy constructor
        cst_cn(const cst_cn& cst) {
            copy(cst);
        }

        //! Move constructor
        cst_cn(cst_cn&& cst) {
            *this = std::move(cst);
        }

        //! Construct CST from cache config
        cst_cn(cache_config& config) {
            {
                auto event = memory_monitor::event("load csa");
                load_from_cache(m_csa, std::string(conf::KEY_CSA) + "_" + util::class_to_hash(m_csa), config);
            }
            {
                auto event = memory_monitor::event("load lcp");
                cache_config tmp_config(false, config.dir, config.id, config.file_map);
                construct_lcp(m_lcp, *this, tmp_config);
                config.file_map = tmp_config.file_map;
                typename lcp_type::lcp_category tag;
                assign_to_lcp(tag); //added to fix lcp problem when its depends on cst or csa
            }
            {
                auto event = memory_monitor::event("construct NPR");
                m_npr = npr_type(&m_lcp);
                std::cout << "npr created" << std::endl;
            }
        }

        //! Swap method for cst_cn
        void
        swap(cst_cn& cst) {
            if (this != &cst) {
                m_csa.swap(cst.m_csa);
                swap_lcp(m_lcp, cst.m_lcp, *this, cst);
                swap_npr(m_npr, cst.m_npr, *this, cst);
            }
        }

        //! Assignment Operator.
        cst_cn& operator=(const cst_cn& cst) {
            if (this != &cst)
                copy(cst);
            return *this;
        }

        //! Assignment Move Operator.
        cst_cn operator=(cst_cn&& cst) {
            if (this != &cst) {
                m_csa = std::move(cst.m_csa);
                move_lcp(m_lcp, cst.m_lcp, *this);
                move_npr(m_npr, cst.m_npr, *this);
            }
            return *this;
        }

        //! Serialize to a stream.
        size_type
        serialize(std::ostream& out, structure_tree_node* v=nullptr, std::string name="") const {
            structure_tree_node* child = structure_tree::add_child(v, name, util::class_name(*this));
            size_type written_bytes = 0, csa_size = 0, lcp_size = 0, npr_size = 0;
            csa_size += m_csa.serialize(out, child, "csa");
            lcp_size += m_lcp.serialize(out, child, "lcp");
            npr_size += m_npr.serialize(out, child, "npr");
            written_bytes += csa_size + lcp_size + npr_size;
            //std::cout << "CSA uses: " << (csa_size * 8.0) / m_lcp.size() << "n bits" << std::endl;
            //std::cout << "LCP uses: " << (lcp_size * 8.0) / m_lcp.size() << "n bits" << std::endl;
            //std::cout << "NPR uses: " << (npr_size * 8.0) / m_lcp.size() << "n bits" << std::endl;
            structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }

        //! Load from a stream.
        void
        load(std::istream& in) {
            size_type n_levels;
            m_csa.load(in);
            load_lcp(m_lcp, in, *this);
            typename lcp_type::lcp_category tag;
            assign_to_lcp(tag); //just in case that the assigning of the lcp does not work
            m_npr.load(in, &m_lcp);
        }

        //! Number of leaves of the suffix tree.
        size_type
        size() const {
            return m_csa.size();
        }

        //! Returns the largest size that cst_cn can ever have.
        static size_type
        max_size() {
            return t_csa::max_size();
        }

        //! Returns if the data structure is empty.
        bool
        empty() const {
            return m_csa.empty();
        }

        //! Returns a const_iterator to the first element of a depth first traversal of the tree.
        const_iterator
        begin() const {
            if (m_lcp.size() == 0) {
                return end();
            }
            return const_iterator(this, root(), false, true);
        };

        //! Returns a const_iterator to the first element of a depth first traversal of the
        // subtree rooted at node v.
        const_iterator
        begin(const node_type& v) const {
            if (0 == m_lcp.size() and root() == v)
                return end();
            return const_iterator(this, v, false, true);
        }

        //! Returns a const_iterator to the element after the last element of a depth first traversal
        // of the tree.
        const_iterator
        end() const {
            return const_iterator(this, root(), true, false);
        }

        //! Returns a const_iterator to the element past the end of a depth first traversal of the
        // subtree rooted at node v.
        const_iterator
        end(const node_type& v) const {
            if (root() == v)
                return end();
            return ++const_iterator(this, v, true, true);
        }


        //! Returns the root of the suffix tree.
        node_type
        root() const {
            return node_type(0, m_csa.size() - 1);
        }

        //! Returns true iff node v is a leaf.
        bool
        is_leaf(node_type v) const {
            return v.first == v.second;
        }

        //! Return the i-th leaf (1-based from left to right) of the suffix tree.
        node_type
        select_leaf(size_type i) const {
            assert(i > 0 and i <= m_csa.size());
            return node_type(i - 1, i - 1);
        }

        //! Calculates the number of leaves in the subtree rooted at node v.
        size_type
        size(const node_type& v) const {
            return v.second - v.first +1;
        }

        //! Calculates the leftmost leaf in the subtree rooted at node v.
        node_type
        leftmost_leaf(const node_type& v) const {
            return node_type(v.first, v.first);
        }

        //! Calculates the rightmost leaf in the subtree rooted at node v.
        node_type
        rightmost_leaf(const node_type v) const {
            return node_type(v.second, v.second);
        }

        //! Returns the leftmost leaf (left boundary) of a node.
        leaf_type
        lb(node_type v) const {
            return v.first;
        }

        //! Returns the rightmost leaf (right boundary) of a node.
        leaf_type
        rb(node_type v) const {
            return v.second;
        }

        //! Calculate the parent node of a node v.
        node_type
        parent(const node_type& v) const {
            size_type lcp_p_pos;
            //get the lcp value that represent the node
            if (v.second == m_lcp.size() - 1 or m_lcp[v.first] > m_lcp[v.second + 1])
                lcp_p_pos = v.first;
            else
                lcp_p_pos = v.second + 1; //in general this is the first lcp of the neighbour
            return get_node(lcp_p_pos);
        }

        //! Return a proxy object which allows iterating over the children of a node
        cst_node_child_proxy<cst_cn>
        children(const node_type& v) const {
            return cst_node_child_proxy<cst_cn>(this, v);
        }

        //! Returns the next sibling of node v.
        node_type
        sibling(const node_type& v) const {
            size_type l = v.second + 1, r, lcp_si;
            node_type p = parent(v);
            if (v.second >= p.second) //no more siblings
                return root();
            if (l == p.second)  //last sibling is a leaf
                return node_type(p.second, p.second);
            else {
                r = m_npr.fwd_nsv(l + 1, m_lcp[l] + 1, lcp_si) - 1;
                return node_type(l, r);
            }
        }

        //! Get the i-th child of a node v.
        node_type
        select_child(const node_type& v, size_type i) const {
            assert(i > 0);
            if (is_leaf(v))   //no child
                return root();
            size_type lcp_value, left_margin, aux_lcp;
            size_type left, right;
            //every internal node of the tree must have at least two children
            left_margin = m_npr.rmq(v.first + 1, v.second, lcp_value);
            node_type ch(v.first, left_margin - 1); //get the first child
            --i;
            while (i > 0) {
                left = ch.second + 1;
                if (ch.second >= v.second) //no more siblings
                    return root();
                if (left == v.second)  //last sibling is a leaf
                    right = left;
                else
                    right = m_npr.fwd_nsv(left + 1, lcp_value + 1, aux_lcp) - 1;
                ch = node_type(left, right);
                --i;
            }
            return ch;
        }

        //! Get the number of children of a node v.
        size_type
        degree(const node_type& v) const {
            size_type count = 0;
            if (is_leaf(v))  // if v is a leave, v has no child
                return 0;
            size_type lcp_value, left_margin, aux_lcp;
            size_type left, right;
            left_margin = m_npr.rmq(v.first + 1, v.second, lcp_value);
            node_type ch(v.first, left_margin - 1); //get the first child
            while (ch != root()) {
                ++count;
                left = ch.second + 1;
                if (ch.second >= v.second) //no more siblings
                    ch = root();
                else {
                    if (left == v.second)  //last sibling is a leaf
                        right = left;
                    else
                        right = m_npr.fwd_nsv(left + 1, lcp_value + 1, aux_lcp) - 1;
                    ch = node_type(left, right);
                }
            }
            return count;
        }

        //! Get the child w of node v which edge label (v,w) starts with character c.
        node_type
        child(const node_type& v, const char_type c, size_type& char_pos) const {
            if (is_leaf(v))  // if v is a leaf, v has no child
                return root();
            //check firs if the letter is a valid letter using the csa
            comp_char_type cc = m_csa.char2comp[c];
            if (cc==0 and c!=0)
                return root();
            size_type char_ex_max_pos = m_csa.C[((size_type)1)+cc];
            size_type char_inc_min_pos = m_csa.C[cc];
            size_type d = depth(v), left_margin = 0, lcp_value, aux_lcp;
            //(1) check the first child
            node_type v_child, last_child;
            char_pos = get_char_pos(v.first, d, m_csa); //first lex.order character in the interval
            if (char_pos >= char_ex_max_pos)
                return root();
            else {
                v_child = select_child(v, 1);
                left_margin = m_npr.rmq(v.first + 1, v.second, lcp_value);
                v_child = node_type(v.first, left_margin - 1); //get the first child
                if (char_pos >= char_inc_min_pos)
                    return v_child;
            }
            //(2) check the last child
            char_pos = get_char_pos(v.second, d, m_csa); //last lex.order character of the interval
            if (char_pos < char_inc_min_pos)
                return root();
            else{
                last_child = node_type(m_npr.bwd_psv(v.second, lcp_value + 1, aux_lcp), v.second);
                if (char_pos < char_ex_max_pos)
                    return last_child;
            }
            //(3) binary search for c in the children [2..last_child)
            //extract all the other children
            left_margin = last_child.first - 1;
            std::vector<node_type> v_children;
            size_type left, right;
            while (v_child.second != left_margin) {  //get the children of v
                left = v_child.second + 1;
                if (left == left_margin)  //the sibling is a leaf
                    right = left_margin;
                else
                    right = m_npr.fwd_nsv(left + 1, lcp_value + 1, aux_lcp) - 1;
                v_child = node_type(left, right);
                v_children.push_back(v_child);
            }
            size_type l_bound = 0, r_bound = v_children.size(), mid;
            while (l_bound < r_bound) {
                mid = (l_bound + r_bound) >> 1;
                char_pos = get_char_pos(v_children[mid].first, d, m_csa);
                if (char_inc_min_pos > char_pos)
                    l_bound = mid+1;
                else if (char_ex_max_pos <= char_pos)
                    r_bound = mid;
                else //found child
                    return v_children[mid];
            }
            return root(); // not found
        }



        //! Get the child w of node v which edge label (v,w) starts with character c.
        node_type
        child(const node_type& v, const char_type c) const {
            size_type char_pos;
            return child(v, c, char_pos);
        }

        //! Returns the d-th character (1-based indexing) of the edge-label pointing to v.
        // we assume that d is never the root and that the inputs are always good
        char_type
        edge(const node_type& v, size_type d) const {
            assert(1 <= d);
            assert(d <= depth(v));
            size_type char_pos = get_char_pos(v.first, d - 1, m_csa);
            return m_csa.F[char_pos];
        }

        //! Returns true iff v is an ancestor of w.
        bool
        ancestor(node_type v, node_type w) const {
            return (v.first <= w.first) and (v.second >= w.second);
        }

        //! Calculates the Lower Common Ancestor of two nodes `v` and `w`
        node_type
        lca(node_type v, node_type w) const {
            size_type k, lcp_value;
            if (ancestor(v, w))
                return v;
            if (ancestor(w, v))
                return w;
            if (v.second < w.first)
                k = m_npr.rmq(v.second + 1, w.first, lcp_value);
            else //w.second < v.first
                k = m_npr.rmq(w.second + 1, v.first, lcp_value);
            return get_node(k);
        }

        //! Returns the string depth of node v.
        size_type
        depth(const node_type& v) const {
            size_type k, val;
            if (is_leaf(v)) {
                return size() - m_csa[v.first];
            }
            else if (v == root()) {
                return 0;
            }
            else {
                k = npr.rmq(v.first + 1, v.second, val);
                return val;
            }
        }

        //! Returns the node depth of node v
        size_type
        node_depth(node_type v) const {
            size_type d = 0;
            while (v != root()) {
                ++d;
                v = parent(v);
            }
            return d;
        }

        //! Compute the suffix link of node v.
        node_type
        sl(const node_type& v) const {
            size_type x, y, k, lcp_value;
            if (v == root())
                return root();
            else if(is_leaf(v)) {
                x = m_csa.psi[v.first];
                return node_type(x, x);
            }
            else {
                x = m_csa.psi[v.first];
                y = m_csa.psi[v.second];
                if (x < y)
                    k = m_npr.rmq(x + 1, y, lcp_value);
                else
                    k = m_npr.rmq(y + 1, x, lcp_value);
            }
            return get_node(k);
        }

        //! Computes the Weiner link of node v and character c.
        node_type
        wl(const node_type& v, const char_type c) const {
            size_type l, r;
            std::tie(l, r) = v;
            if (l == r) // no WL
                return root();
            sdsl::backward_search(m_csa, l, r, c, l, r);
            return node_type(l, r);
        }

        //! Computes the suffix number of a leaf node v.
        size_type
        sn(const node_type& v) const {
            assert(is_leaf(v));
            return m_csa[v.first];
        }

        //! Computes a unique identification number for a node of the suffix tree in the range [0..nodes()-1]
        size_type
        id(const node_type& v) const {
            size_type val = v.first;
            size_type is_second = 0;
            size_type n = m_lcp.size();
            if (is_leaf(v))
                return val;
            else {
                if (v == root() or (v.second != n - 1  and m_lcp[v.first] < m_lcp[v.second + 1])) {
                    val = v.second;
                    is_second = 1;
                }
                //idea: shift the result in one bit and add the info if it was v.i o v.j
                val = ((n + val) << 1) || is_second;
                return n + val;
            }
        }

        //! Computes the node for such that id(v)=id.
        node_type
        inv_id(size_type id) {
            size_type is_second = 1, i, j, lcp_value;
            size_type n = m_lcp.size();
            if (id < n) //is a leaf
                return node_type(id, id);
            else {
                id -= n;
                if (id ==  n - 1)
                    return root();
                else {
                    is_second = id && is_second;
                    i =  (id >> 1) - n;
                    if (is_second) {
                        j = i - 1;
                        i = m_npr.bwd_psv(j - 1, m_lcp[j] + 1, lcp_value);
                        if (i == m_lcp.size())
                            i = 0;
                    }
                    else
                        j = m_npr.fwd_nsv(i + 1, m_lcp[i] + 1, lcp_value) - 1;
                    return node_type(i,j);
                }
            }
        }

        //! Get the number of nodes of the suffix tree.
        size_type
        nodes() const {
            return 0; //not supported yet (idea: pre compute and store value into m_nodes--> increase construction time)
        }

        //! Get the node in the suffix tree which corresponds to the sa-interval [lb..rb]
        node_type
        node(size_type lb, size_type rb) const {
            return node_type(lb, rb);
        }

        //! Get the lower ancestor of the node v, w, such that the depth(w) <= d
        node_type
        laqs(const node_type& v, size_type d) const {
            if (d == 0)
                return root();
            size_type l, r, lcp_value;
            l = m_npr.bwd_psv(v.first, d + 1, lcp_value);
            if (l == m_lcp.size())
                l = 0;
            r = m_npr.fwd_nsv(v.second, d + 1, lcp_value) - 1;
            return node_type(l,r);
        }

        //! Get the lowest ancestor of a node v, w, such that the node_depth(w) <= d
        node_type
        laqt(const node_type& v, size_type d) const {
            if (d == 0)
                return root();
            size_type node_d, diff, node_sd;
            node_type res, aux;
            res = laqs(v, d);
            node_d = node_depth(res); //worse case d parent
            diff = d - node_d;
            while (diff != 0 and res != v) {
                node_sd = depth(res); // this would speed the process
                aux = laqs(v, node_sd + diff);
                while (aux != res) { //update node_d
                    aux = parent(aux);
                    ++node_d;
                }
                res = aux;
                diff = d - node_d;
            }
            return res;
        }

    private:

        void
        copy(const cst_cn& cst) {
            m_csa = cst.m_csa;
            copy_lcp(m_lcp, cst.m_lcp, *this);
            copy_npr(m_npr, cst.m_npr, *this);
        }

        node_type
        get_node(size_type pos) const {
            size_type l, r, lcp_p;
            l = m_npr.psv(pos, lcp_p); //left border
            r = m_npr.nsv(pos, lcp_p) - 1; //right border
            if (l == m_lcp.size())
                l = 0;
            return node_type(l,r);
        }

        node_type
        select_last_child(const node_type& v) const {
            if (is_leaf(v))   //no child
                return root();
            size_type lcp_value;
            size_type min_lcp = m_npr.rmq(v.first + 1, v.second, lcp_value);
            node_type ch(m_npr.bwd_psv(v.second, lcp_value + 1, min_lcp), v.second);
            return ch;
        }

        void assign_to_lcp(lcp_plain_tag) {}
        void assign_to_lcp(lcp_permuted_tag) { m_lcp.set_csa(&(csa));}
        void assign_to_lcp(lcp_tree_compressed_tag) { m_lcp.set_cst(*this);}
        void assign_to_lcp(lcp_tree_and_lf_compressed_tag) {m_lcp.set_cst(*this);}

    };

}

#endif //CST_CN_HPP
