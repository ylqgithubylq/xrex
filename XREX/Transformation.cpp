#include "XREX.hpp"

#include "Transformation.hpp"


Transformation::Transformation()
	: position_(floatV3::Zero), orientation_(floatM44::Identity), scaling_(floatV3(1.0f, 1.0f, 1.0f))
{
}


Transformation::~Transformation()
{
}

