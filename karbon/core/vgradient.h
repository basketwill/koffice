/* This file is part of the KDE project
   Copyright (C) 2002, The Karbon Developers
*/

#ifndef __VGRADIENT_H__
#define __VGRADIENT_H__

#include <qvaluelist.h>
#include <koPoint.h>
#include "vcolor.h"


class QDomElement;


class VGradient
{
public:
	enum VGradientType
	{
		linear = 0,
		radial = 1,
		conic  = 2
	};

	enum VGradientRepeatMethod
	{
		none    = 0,
		reflect = 1,
		repeat  = 2
	};

	struct VColorStop
	{
		VColor color;

		// relative position of color point (0.0-1.0):
		float rampPoint;

		// relative position of midpoint (0.0-1.0)
		// between two ramp points. ignored for last VColorStop.
		float midPoint;
	};

	VGradient( VGradientType type = linear );

	VGradientType type() const { return m_type; }
	void setType( VGradientType type ) { m_type = type; }

	VGradientRepeatMethod repeatMethod() const { return m_repeatMethod; }
	void setRepeatMethod( VGradientRepeatMethod repeatMethod ) { m_repeatMethod = repeatMethod; }

	QValueList<VColorStop>& colorStops() { return m_colorStops; }
	void addStop( const VColor &color, float rampPoint, float midPoint );

	KoPoint origin() const { return m_origin; }
	void setOrigin( const KoPoint &origin ) { m_origin = origin; }

	KoPoint vector() const { return m_vector; }
	void setVector( const KoPoint &vector ) { m_vector = vector; }

	void save( QDomElement& element ) const;
	void load( const QDomElement& element );

	void transform( const QWMatrix& m );

private:
	VGradientType m_type;
	VGradientRepeatMethod m_repeatMethod;

	QValueList<VColorStop> m_colorStops;

	// coordinates:
	KoPoint m_origin;
	KoPoint m_vector;
};

#endif
