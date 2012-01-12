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

#include "frame_producer.h"

#include <common/no_copy.h>
#include <common/forward.h>
#include <common/memory/safe_ptr.h>
#include <common/concurrency/target.h>

#include <boost/property_tree/ptree_fwd.hpp>

#include <functional>

FORWARD2(caspar, diagnostics, class graph);
FORWARD1(boost, template<typename> class unique_future);

namespace caspar { namespace core {
	
class stage sealed
{
	CASPAR_NO_COPY(stage);
public:	
	typedef target<std::pair<std::map<int, safe_ptr<class basic_frame>>, std::shared_ptr<void>>> target_t;

	stage(const safe_ptr<target_t>& target, const safe_ptr<diagnostics::graph>& graph, const struct video_format_desc& format_desc);
	
	// stage
	
	void apply_transform(int index, const struct frame_transform& transform, unsigned int mix_duration = 0, const std::wstring& tween = L"linear");
	void apply_transform(int index, const std::function<struct frame_transform(struct frame_transform)>& transform, unsigned int mix_duration = 0, const std::wstring& tween = L"linear");
	void clear_transforms(int index);
	void clear_transforms();

	void spawn_token();
			
	void load(int index, const safe_ptr<struct frame_producer>& producer, bool preview = false, int auto_play_delta = -1);
	void pause(int index);
	void play(int index);
	void stop(int index);
	void clear(int index);
	void clear();	
	void swap_layers(const safe_ptr<stage>& other);
	void swap_layer(int index, int other_index);
	void swap_layer(int index, int other_index, const safe_ptr<stage>& other);
	
	boost::unique_future<std::wstring>						call(int index, bool foreground, const std::wstring& param);
	boost::unique_future<safe_ptr<struct frame_producer>>	foreground(int index);
	boost::unique_future<safe_ptr<struct frame_producer>>	background(int index);

	boost::unique_future<boost::property_tree::wptree> info() const;
	boost::unique_future<boost::property_tree::wptree> info(int layer) const;
	
	void set_video_format_desc(const struct video_format_desc& format_desc);

private:
	struct impl;
	safe_ptr<impl> impl_;
};

}}