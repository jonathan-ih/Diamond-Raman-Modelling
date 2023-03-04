#include "laser.h"

Laser::Laser(double intensity) : m_intensity(intensity) {}

Laser::Laser(const LaserSettings &laser_settings) : m_intensity(laser_settings.intensity) {}