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

/*! \file npr.hpp
    \brief npr.hpp contains classes for npr information.
    \author Rodrigo Canovas
*/
#ifndef CST_CN_NPR_H
#define CST_CN_NPR_H

#include "npr_support_cn.h"
#include "npr_support_cnr.h"

namespace cstds{

    template<class t_npr, class t_cst>
    void
    swap_npr(t_npr& npr1, t_npr& npr2, const t_cst& cst1, const t_cst& cst2) {
        npr1.swap(npr2);
        npr1.set_lcp(&(cst1.lcp)); //assume that the lcp had been swap already
        npr2.set_lcp(&(cst2.lcp));
    }

    template<class t_npr, class t_cst>
    void
    copy_npr(t_npr& npr, const t_npr& npr_c, const t_cst& cst) {
        npr = npr_c;
        npr.set_lcp(&(cst.lcp));
    }

    template<class t_npr, class t_cst>
    void move_npr(t_npr& npr, t_npr& npr_c, const t_cst& cst) {
        npr = std::move(npr_c);
        npr.set_lcp(&(cst.lcp));
    }

}

#endif //CST_CN_NPR_H
