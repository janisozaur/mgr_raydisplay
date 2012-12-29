#ifndef BLOB_H
#define BLOB_H

#include <opencv2/core/core.hpp>

enum {
	BLOB_NULL,
	BLOB_DOWN,
	BLOB_MOVE,
	BLOB_UP
}; // event types

struct point {
	double x, y;
};

class Blob {
public:
	cv::Point2i location, origin; // current location and origin for defining a drag vector
	cv::Point2i min, max;			// to define our axis-aligned bounding box
	int event;				// event type: one of BLOB_NULL, BLOB_DOWN, BLOB_MOVE, BLOB_UP
	bool tracked;			// a flag to indicate this blob has been processed
};

#endif
