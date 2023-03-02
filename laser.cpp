#include "laser.h"

Laser::Laser(double m_intensity) : m_intensity(m_intensity) {}

Laser::Laser(const LaserSettings laser_settings) : m_intensity(laser_settings.intensity) {}