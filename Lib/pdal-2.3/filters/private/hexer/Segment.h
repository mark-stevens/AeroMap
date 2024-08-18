#ifndef SEGMENT_H
#define SEGMENT_H

#include <iostream>
#include <stdlib.h>
#include <vector>

#include "Mathpair.h"

namespace hexer
{
	class Hexagon;
	class HexGrid;

	class Segment
	{
	public:
		Segment() : m_hex(NULL), m_side(0)
		{}

		Segment(Hexagon* h, int side) : m_hex(h), m_side(side)
		{}

		Hexagon* hex()
		{
			return m_hex;
		}

		int side()
		{
			return m_side;
		}

		bool horizontal()
		{
			return (m_side == 0 || m_side == 3);
		}

		void normalize(HexGrid* grid);
		bool possibleRoot(HexGrid* grid);

		Segment rightAntiClockwise(HexGrid* grid);
		Segment leftAntiClockwise(HexGrid* grid);
		Segment rightClockwise(HexGrid* grid);
		Segment leftClockwise(HexGrid* grid);
		Point startPos(HexGrid* grid) const;
		Point endPos(HexGrid* grid) const;

		friend bool operator == (const Segment& s1, const Segment& s2);
		friend bool operator != (const Segment& s1, const Segment& s2);
		friend std::ostream& operator << (std::ostream& os, const Segment& s);

	private:
		Point pos(HexGrid* grid, const Point& offset) const;

		/// Hexagon who's side is the segment.
		Hexagon* m_hex;
		/// Which side of the hexagon.
		int m_side;
	};
}

#endif // #ifndef SEGMENT_H
