#include "Tracker.h"

Tracker::Tracker(double min_area, double max_radius) : min_area(min_area), max_radius(max_radius)
{
}

Tracker::~Tracker()
{
}

void Tracker::trackBlobs(cv::Mat &mat, bool history)
{
	double x, y, min_x, min_y, max_x, max_y;
	Blob temp;

	// convert our OpenCV matrix object to one of type IplImage
	img = mat;

	// cvblobslib blob extraction
	blob_result = CBlobResult(&img, NULL, 0);
	blob_result.Filter(blob_result, B_EXCLUDE, CBlobGetArea(), B_LESS, min_area); // filter blobs with area less than min_area units

	// clear the blobs from two frames ago
	blobs_previous.resize(0);

	// before we populate the blobs vector with the current frame, we need to store the live blobs in blobs_previous
	for (int i = 0; i < blobs.size(); i++) {
		if (blobs.at(i).event != BLOB_UP) {
			blobs_previous.push_back(blobs.at(i));
		}
	}

	// populate the blobs vector with the current frame
	blobs.resize(0);
	for (int i = 1; i < blob_result.GetNumBlobs(); i++) {
		current_blob = blob_result.GetBlob(i);

		x     = XCenter(*current_blob);
		y     = YCenter(*current_blob);
		min_x = MinX(*current_blob);
		min_y = MinY(*current_blob);
		max_x = MaxX(*current_blob);
		max_y = MaxY(*current_blob);

		temp.location.x = temp.origin.x = x;
		temp.location.y = temp.origin.y = y;
		temp.min.x = min_x; temp.min.y = min_y;
		temp.max.x = max_x; temp.max.y = max_y;
		temp.event = BLOB_DOWN;

		blobs.push_back(temp);
	}

	// initialize previous blobs to untracked
	for (int i = 0; i < blobs_previous.size(); i++) blobs_previous[i].tracked = false;

	// main tracking loop -- O(n^2) -- simply looks for a blob in the previous frame within a specified radius
	for (int i = 0; i < blobs.size(); i++) {
		for (int j = 0; j < blobs_previous.size(); j++) {
			if (blobs_previous[j].tracked) continue;

			if (sqrt(pow(blobs.at(i).location.x - blobs_previous.at(j).location.x, 2.0) + pow(blobs.at(i).location.y - blobs_previous.at(j).location.y, 2.0)) < max_radius) {
				blobs_previous[j].tracked = true;
				blobs[i].event = BLOB_MOVE;
				blobs[i].origin.x = history ? blobs_previous.at(j).origin.x : blobs_previous.at(j).location.x;
				blobs[i].origin.y = history ? blobs_previous.at(j).origin.y : blobs_previous.at(j).location.y;
			}
		}
	}

	// add any blobs from the previous frame that weren't tracked as having been removed
	for (int i = 0; i < blobs_previous.size(); i++) {
		if (!blobs_previous[i].tracked) {
			blobs_previous[i].event = BLOB_UP;
			blobs.push_back(blobs_previous.at(i));
		}
	}
}

QVector<Blob> &Tracker::getBlobs()
{
	return blobs;
}
