#ifndef DIAMOND_RAMAN_MODELLING_LASER_H
#define DIAMOND_RAMAN_MODELLING_LASER_H

#include "settings.h"

class Laser {
public:
    double m_intensity;

    Laser(double m_intensity);
    Laser(const LaserSettings laser_settings);

};


#endif //DIAMOND_RAMAN_MODELLING_LASER_H
