#ifndef DIAMOND_RAMAN_MODELLING_LASER_H
#define DIAMOND_RAMAN_MODELLING_LASER_H

#include "settings.h"

class Laser {
public:

    Laser(double intensity);
    Laser(const LaserSettings &laser_settings);

    double get_intensity() { return m_intensity ;}
    double get_intensity() const { return m_intensity ;}

private:
    double m_intensity;

};


#endif //DIAMOND_RAMAN_MODELLING_LASER_H
