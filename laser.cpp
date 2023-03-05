#include "laser.h"

Laser::Laser(double intensity) : m_intensity(intensity) {}

Laser::Laser(const Settings &settings) : m_intensity(settings.laser.intensity) {}