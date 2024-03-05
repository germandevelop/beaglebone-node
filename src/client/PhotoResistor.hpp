/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef PHOTO_RESISTOR_H_
#define PHOTO_RESISTOR_H_

#include <cstddef>

class PhotoResistor
{
    public:
        explicit PhotoResistor ();
        PhotoResistor (const PhotoResistor&) = delete;
        PhotoResistor& operator= (const PhotoResistor&) = delete;
        PhotoResistor (PhotoResistor&&) = delete;
        PhotoResistor& operator= (PhotoResistor&&) = delete;
        ~PhotoResistor ();

    public:
        std::size_t readAdcValue () const;
};

#endif // PHOTO_RESISTOR_H_
