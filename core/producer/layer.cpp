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

#include "../stdafx.h"

#include "layer.h"

#include "frame_producer.h"

#include "../frame/draw_frame.h"
#include "../frame/frame_transform.h"

#include <boost/optional.hpp>
#include <boost/thread/future.hpp>

namespace caspar { namespace core {

struct layer::impl
{				
	spl::shared_ptr<frame_producer>	foreground_;
	spl::shared_ptr<frame_producer>	background_;
	int64_t						frame_number_;
	boost::optional<int32_t>	auto_play_delta_;
	bool						is_paused_;

public:
	impl() 
		: foreground_(frame_producer::empty())
		, background_(frame_producer::empty())
		, frame_number_(0)
		, is_paused_(false)
	{
	}
	
	void pause()
	{
		is_paused_ = true;
	}

	void resume()
	{
		is_paused_ = false;
	}

	void load(spl::shared_ptr<frame_producer> producer, const boost::optional<int32_t>& auto_play_delta)
	{		
		background_		 = std::move(producer);
		auto_play_delta_ = auto_play_delta;

		if(auto_play_delta_ && foreground_ == frame_producer::empty())
			play();
	}
	
	void play()
	{			
		if(background_ != frame_producer::empty())
		{
			background_->set_leading_producer(foreground_);
			foreground_		= background_;
			background_		= frame_producer::empty();
			frame_number_	= 0;
			auto_play_delta_.reset();
		}

		resume();
	}
	
	void stop()
	{
		foreground_		= frame_producer::empty();
		frame_number_	= 0;
		auto_play_delta_.reset();

		pause();
	}
		
	spl::shared_ptr<draw_frame> receive(frame_producer::flags flags)
	{		
		try
		{
			if(is_paused_)
				return draw_frame::silence(foreground_->last_frame());
		
			auto frame = receive_and_follow(foreground_, flags.value());
			if(frame == core::draw_frame::late())
				return draw_frame::silence(foreground_->last_frame());

			if(auto_play_delta_)
			{
				auto frames_left = static_cast<int64_t>(foreground_->nb_frames()) - static_cast<int64_t>(++frame_number_) - static_cast<int64_t>(*auto_play_delta_);
				if(frames_left < 1)
				{
					play();
					return receive(flags);
				}
			}
				
			return frame;
		}
		catch(...)
		{
			CASPAR_LOG_CURRENT_EXCEPTION();
			stop();
			return core::draw_frame::empty();
		}
	}
	
	boost::property_tree::wptree info() const
	{
		boost::property_tree::wptree info;
		info.add(L"status",		is_paused_ ? L"paused" : (foreground_ == frame_producer::empty() ? L"stopped" : L"playing"));
		info.add(L"auto_delta",	(auto_play_delta_ ? boost::lexical_cast<std::wstring>(*auto_play_delta_) : L"null"));
		info.add(L"frame-number", frame_number_);

		auto nb_frames = foreground_->nb_frames();

		info.add(L"nb_frames",	 nb_frames == std::numeric_limits<int64_t>::max() ? -1 : nb_frames);
		info.add(L"frames-left", nb_frames == std::numeric_limits<int64_t>::max() ? -1 : (foreground_->nb_frames() - frame_number_ - (auto_play_delta_ ? *auto_play_delta_ : 0)));
		info.add_child(L"foreground.producer", foreground_->info());
		info.add_child(L"background.producer", background_->info());
		return info;
	}
};

layer::layer() : impl_(new impl()){}
layer::layer(const layer& other) : impl_(new impl(*other.impl_)){}
layer::layer(layer&& other) : impl_(std::move(other.impl_)){}
layer& layer::operator=(layer other)
{
	other.swap(*this);
	return *this;
}
void layer::swap(layer& other)
{	
	impl_.swap(other.impl_);
}
void layer::load(spl::shared_ptr<frame_producer> frame_producer, const boost::optional<int32_t>& auto_play_delta){return impl_->load(std::move(frame_producer), auto_play_delta);}	
void layer::play(){impl_->play();}
void layer::pause(){impl_->pause();}
void layer::stop(){impl_->stop();}
spl::shared_ptr<draw_frame> layer::receive(frame_producer::flags flags) {return impl_->receive(flags);}
spl::shared_ptr<frame_producer> layer::foreground() const { return impl_->foreground_;}
spl::shared_ptr<frame_producer> layer::background() const { return impl_->background_;}
boost::property_tree::wptree layer::info() const{return impl_->info();}
}}