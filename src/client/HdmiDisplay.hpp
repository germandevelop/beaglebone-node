/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef HDMI_DISPLAY_H_
#define HDMI_DISPLAY_H_

#include <string>
#include <memory>

typedef struct frame_buffer frame_buffer_t;

class HdmiDisplay
{
    public:
        enum COLOR : std::size_t
        {
            BLACK = 0U,
            WHITE,
            RED,
            GREEN,
            BLUE,
            GRAY,
            COUNT
        };

        struct Text
        {
            std::string text;
            std::string font;
            COLOR color;
            double x, y;
        };

        struct Line
        {
            COLOR color;
            double width;
            double x_0, y_0;
            double x_1, y_1;
        };

        struct Image
        {
            std::string file;
            double x, y;
        };

    public:
        explicit HdmiDisplay ();
        HdmiDisplay (const HdmiDisplay&) = delete;
        HdmiDisplay& operator= (const HdmiDisplay&) = delete;
        HdmiDisplay (HdmiDisplay&&) = delete;
        HdmiDisplay& operator= (HdmiDisplay&&) = delete;
        ~HdmiDisplay ();

    public:
        void enableFrameBuffer ();
        void disableFrameBuffer ();

        void fillBackground (COLOR color) const;
        void drawText (Text text) const;
        void drawLine (Line line) const;
        void drawImage (Image image) const;

    private:
        std::unique_ptr<frame_buffer_t> frameBuffer;

};

#endif // HDMI_DISPLAY_H_
