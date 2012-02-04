/*
* Copyright (c) 2011 Sveriges Television AB <info@casparcg.com>
*
* This file is part of CasparCG (www.casparcg.com).
*
* CasparCG is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* CasparCG is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with CasparCG. If not, see <http://www.gnu.org/licenses/>.
*
* Author: Robert Nagy, ronag89@gmail.com
*/

#pragma once

#include "image/blend_modes.h"

#include <common/forward.h>
#include <common/spl/memory.h>
#include <common/reactive.h>

#include <boost/noncopyable.hpp>
#include <boost/property_tree/ptree_fwd.hpp>

#include <map>

FORWARD1(boost, template<typename> class unique_future);
FORWARD2(caspar, diagnostics, class graph);
FORWARD3(caspar, core, gpu, class accelerator);
FORWARD2(caspar, core, struct write_frame);
FORWARD2(caspar, core, struct pixel_format_desc);

namespace caspar { namespace core {
	
class mixer sealed : boost::noncopyable
{
public:	
	explicit mixer(const spl::shared_ptr<gpu::accelerator>& ogl);
					
	spl::shared_ptr<const struct data_frame> operator()(std::map<int, spl::shared_ptr<class draw_frame>> frames, const struct video_format_desc& format_desc);
	
	void set_blend_mode(int index, blend_mode value);

	boost::unique_future<boost::property_tree::wptree> info() const;

	spl::shared_ptr<core::write_frame> create_frame(const void* tag, const core::pixel_format_desc& desc);
private:
	struct impl;
	spl::shared_ptr<impl> impl_;
};

}}