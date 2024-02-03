/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#include "HdmiDisplay.hpp"

#include <array>

#include <cairomm/cairomm.h>
#include <pangomm.h>

#include "frame_buffer.h"
#include "std_error/std_error.h"


HdmiDisplay::HdmiDisplay ()
{
	this->frameBuffer = std::make_unique<frame_buffer_t>();

	return;
}

HdmiDisplay::~HdmiDisplay () = default;


void HdmiDisplay::enableFrameBuffer ()
{
	std_error_t error;
	std_error_init(&error);

	if (frame_buffer_open(this->frameBuffer.get(), "/dev/fb0", &error) != STD_SUCCESS)
	{
		throw std::runtime_error { error.text };
	}

	return;
}

void HdmiDisplay::disableFrameBuffer ()
{
	std_error_t error;
	std_error_init(&error);

	if (frame_buffer_close(this->frameBuffer.get(), &error) != STD_SUCCESS)
	{
		throw std::runtime_error { error.text };
	}

	return;
}

void HdmiDisplay::fillBackground (HdmiDisplay::COLOR color) const
{
	std::array<std::array<double, 3U>, HdmiDisplay::COLOR::COUNT> colors;
	colors[HdmiDisplay::COLOR::BLACK]	= { 0.0, 0.0, 0.0 }; // Red, Green, Blue
	colors[HdmiDisplay::COLOR::WHITE]	= { 1.0, 1.0, 1.0 };
	colors[HdmiDisplay::COLOR::RED]		= { 1.0, 0.0, 0.0 };
	colors[HdmiDisplay::COLOR::GREEN]	= { 0.0, 1.0, 0.0 };
	colors[HdmiDisplay::COLOR::BLUE]	= { 0.0, 0.0, 1.0 };
	colors[HdmiDisplay::COLOR::GRAY]	= { 0.5, 0.5, 0.5 };

	frame_buffer_data_t fb_data;
	frame_buffer_get_data(this->frameBuffer.get(), &fb_data);

	const int stride = Cairo::ImageSurface::format_stride_for_width(Cairo::Surface::Format::RGB16_565, fb_data.width);

	auto surface = Cairo::ImageSurface::create(fb_data.buffer,
												Cairo::Surface::Format::RGB16_565,
												fb_data.width,
												fb_data.height,
												stride);

	auto context = Cairo::Context::create(surface);

	context->set_source_rgb(colors[color][0], colors[color][1], colors[color][2]);
	context->paint();

	return;
}

void HdmiDisplay::drawText (HdmiDisplay::Text text) const
{
	std::array<std::array<double, 3U>, HdmiDisplay::COLOR::COUNT> colors;
	colors[HdmiDisplay::COLOR::BLACK]	= { 0.0, 0.0, 0.0 }; // Red, Green, Blue
	colors[HdmiDisplay::COLOR::WHITE]	= { 1.0, 1.0, 1.0 };
	colors[HdmiDisplay::COLOR::RED]		= { 1.0, 0.0, 0.0 };
	colors[HdmiDisplay::COLOR::GREEN]	= { 0.0, 1.0, 0.0 };
	colors[HdmiDisplay::COLOR::BLUE]	= { 0.0, 0.0, 1.0 };
	colors[HdmiDisplay::COLOR::GRAY]	= { 0.5, 0.5, 0.5 };

	frame_buffer_data_t fb_data;
	frame_buffer_get_data(this->frameBuffer.get(), &fb_data);

	const int stride = Cairo::ImageSurface::format_stride_for_width(Cairo::Surface::Format::RGB16_565, fb_data.width);

	auto surface = Cairo::ImageSurface::create(fb_data.buffer,
												Cairo::Surface::Format::RGB16_565,
												fb_data.width,
												fb_data.height,
												stride);

	auto context = Cairo::Context::create(surface);

	context->move_to(text.x, text.y);
	context->set_source_rgb(colors[text.color][0], colors[text.color][1], colors[text.color][2]);

	auto layout = Pango::Layout::create(context);

	layout->set_font_description(Pango::FontDescription { text.font });
	layout->set_text(text.text);
	layout->show_in_cairo_context(context);

	return;
}

void HdmiDisplay::drawLine (HdmiDisplay::Line line) const
{
	std::array<std::array<double, 3U>, HdmiDisplay::COLOR::COUNT> colors;
	colors[HdmiDisplay::COLOR::BLACK]	= { 0.0, 0.0, 0.0 }; // Red, Green, Blue
	colors[HdmiDisplay::COLOR::WHITE]	= { 1.0, 1.0, 1.0 };
	colors[HdmiDisplay::COLOR::RED]		= { 1.0, 0.0, 0.0 };
	colors[HdmiDisplay::COLOR::GREEN]	= { 0.0, 1.0, 0.0 };
	colors[HdmiDisplay::COLOR::BLUE]	= { 0.0, 0.0, 1.0 };
	colors[HdmiDisplay::COLOR::GRAY]	= { 0.5, 0.5, 0.5 };

	frame_buffer_data_t fb_data;
	frame_buffer_get_data(this->frameBuffer.get(), &fb_data);

	const int stride = Cairo::ImageSurface::format_stride_for_width(Cairo::Surface::Format::RGB16_565, fb_data.width);

	auto surface = Cairo::ImageSurface::create(fb_data.buffer,
												Cairo::Surface::Format::RGB16_565,
												fb_data.width,
												fb_data.height,
												stride);

	auto context = Cairo::Context::create(surface);

	context->move_to(line.x_0, line.y_0);
	context->line_to(line.x_1, line.y_1);
	context->set_source_rgb(colors[line.color][0], colors[line.color][1], colors[line.color][2]);
	context->set_line_width(line.width);
	context->stroke();

	return;
}

void HdmiDisplay::drawImage (HdmiDisplay::Image image) const
{
	frame_buffer_data_t fb_data;
	frame_buffer_get_data(this->frameBuffer.get(), &fb_data);

	const int stride = Cairo::ImageSurface::format_stride_for_width(Cairo::Surface::Format::RGB16_565, fb_data.width);

	auto surface = Cairo::ImageSurface::create(fb_data.buffer,
												Cairo::Surface::Format::RGB16_565,
												fb_data.width,
												fb_data.height,
												stride);

	auto imageSurface = Cairo::ImageSurface::create_from_png(image.file);

	auto context = Cairo::Context::create(surface);
	context->set_source(imageSurface, image.x, image.y);

	context->paint();

	return;
}
