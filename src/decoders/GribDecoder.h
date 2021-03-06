/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/*! \file GribDecoder.h
    \brief Definition of the Template class GribDecoder.

    Magics Team - ECMWF 2004

    Started: Tue 16-Mar-2004

    Changes:

*/

#ifndef GribDecoder_H
#define GribDecoder_H

#include "magics.h"
#include "MagException.h"


#include "Decoder.h"
#include "Data.h"
#include "GribDecoderAttributes.h"
#include "UserPoint.h"


#include "GribLoopAttributes.h"

#include "grib_api.h"

#include "GribInterpretor.h"


namespace magics {
class GribDecoder;
struct MatchCriteria
{
    MatchCriteria() {}
    virtual ~MatchCriteria() {}
    virtual bool verify(const GribDecoder&, const string&, const string&) { return false; }
};

class GribMagException : public MagicsException
{
public:
	 GribMagException( const string& why ):
		MagicsException("Grib API error message: " +  why){}
};
class GribFileMagException : public MagicsException
{
public:
	GribFileMagException(const string& file, int index)
	{
		ostringstream s;
		s << "Grib decoding failed: field " << index << " in " << file << endl;
		what_ = s.str();
	}
};

class GribLoop;



class GribDecoder:
    public Decoder,
    public Data,
    public GribDecoderAttributes
{
public:
	GribDecoder();
	virtual ~GribDecoder();

	enum InterpolateMethod { nearest, nearest_valid, interpolate };

	// implements BaseSceneObject interface
	virtual void set(const map<string, string>& params) { GribDecoderAttributes::set(params); }
	virtual void set(const XmlNode& node) { GribDecoderAttributes::set(node); }
	void set(const GribLoop&, int);
	// implements Decoder interface
	void decode();
	void decode1D();  // RV MF
	void decode2D();
	void decode(const Transformation&);
	void decode2D(const Transformation&);
	void decodeRaster(const Transformation&);
	void decodePoints();
	void release();
	void newPoint(const Transformation&, double, double, double, double, double, vector<CustomisedPoint*>&, double);
	bool verify(const string& where) const;
	MatrixHandler& direction();
	// Data Interface : info for the layer managment!
	 string layerId()  { decode(); return layerId_; }
	 string name() {

	 	 return name_;
	 }
	 const DateTime& from() {
	 return from_; }
	 const DateTime& to()  {  return to_; }

	string title() {
		return title_;
	}

	static void scale(const string&, double&, double&);

	InterpolateMethod interpolateMethod() const {
		if ( magCompare(interpolation_method_, "interpolate") )
			return interpolate;
		if ( magCompare(interpolation_method_, "nearest") )
			return nearest;
		if ( magCompare(interpolation_method_, "nearest_valid") )
			return nearest_valid;
		return interpolate;
	}
	int missingFill() const { return missing_fill_count_; }
	bool getExpver() const { return expver_; }
	void version();


	// implements Decoder
		void visit(AnimationRules&);
		void visit(MetaDataCollector&);
		void visit(MagnifierCollector&);
		void visit(ValuesCollector&);
		void visit(Transformation&);
		void visit(MetaDataVisitor&);
		void ask(MetaDataCollector&);

		 const DateDescription& timeStamp();
		 const LevelDescription& level() ;

	// implements Decoder
	void visit(TextVisitor&);

	PointsHandler& points()
	{
		decodePoints();
		pointsHandlers_.push_back(new PointsHandler(points_));
		return *(pointsHandlers_.back());
	}
	PointsHandler& points(const Transformation& transformation)
		{
			decodePoints();
			pointsHandlers_.push_back(new BoxPointsHandler(points_, transformation, true));
			return *(pointsHandlers_.back());
		}
	PointsHandler& points(const Transformation& transformation, bool all) {
		decodePoints();
		pointsHandlers_.push_back(new BoxPointsHandler(points_, transformation, !all));
		return *(pointsHandlers_.back());
	}

	MatrixHandler& matrix()
	{

// RV MF
		decode1D();
//		decode();
		matrixHandlers_.push_back(new MatrixHandler(*matrix_));
		return *(matrixHandlers_.back());
	}
	MatrixHandler& matrix(const Transformation& transformation)
		{


			decode(transformation);
			matrixHandlers_.push_back(new MatrixHandler(*matrix_));
			return *(matrixHandlers_.back());
		}

	void setPath(const string& path) { file_name_ = path; }

	RasterData&  raster(const Transformation& transformation)
	{
		decodeRaster(transformation);
		return raster_;
	}
	void customisedPoints(const Transformation& t, const std::set<string>& n, CustomisedPointsList& out, bool all)
	{

	}

	void customisedPoints(const AutomaticThinningMethod&, const Transformation&, const std::set<string>&, CustomisedPointsList&);
	void customisedPoints(const BasicThinningMethod&, const Transformation&, const std::set<string>&, CustomisedPointsList& );
	void customisedPoints(const Transformation& t,  CustomisedPointsList& out, double xpts, double ypts, double gap);


	virtual grib_handle* open(grib_handle*, bool sendMsg = true);
	virtual void openFirstComponent();
	virtual void openSecondComponent();
	virtual void openThirdComponent();
	virtual void readColourComponent();

	grib_handle*  id() const { if (!handle_) const_cast<GribDecoder*>(this)->decode(); return handle_; }
	long      getLong(const string&,   bool warnIfKeyAbsent = true) const;
	string    getString(const string&, bool warnIfKeyAbsent = true) const;
	double    getDouble(const string&, bool warnIfKeyAbsent = true) const;
    void      setDouble(const string&, double) const;

    string    getstring(const string&, bool warnIfKeyAbsent = true, bool cache= true) const;

	void      read(Matrix **matrix, Matrix ** matrix2 = NULL);
	void      read(Matrix **matrix, const Transformation&);
	bool      id(const string&, const string&) const;

	grib_nearest* nearest_point_handle(bool keep);
    void nearestGridpoints(double *inlats, double *inlons, double *outlats, double *outlons, double *values, double *distances, int nb, string &representation);

	grib_handle*  uHandle(string&);
	grib_handle*  vHandle(string&);

	grib_handle*  uHandle() const { return component1_ ; }
	grib_handle*  vHandle() const { return component2_ ; };

	grib_handle*  cHandle(string&);

	double uComponent(int);
	double vComponent(int);
	void uComponent();
	void vComponent();

	grib_handle*  handle() const { return handle_; }
        void initInfo();

protected:
	//! Method to print string about this class on to a stream of type ostream (virtual).
	 virtual void print(ostream&) const;

	void handle(grib_handle*);

	mutable Matrix*     matrix_;
	mutable double*     xValues_;
	mutable double*     yValues_;
	mutable Matrix*     xComponent_;
	mutable Matrix*     yComponent_;
	mutable Matrix*     colourComponent_;
	mutable RasterData raster_;
	mutable PointsList points_;


	bool thinning_debug_;

	mutable map<string, string> sKeys_;
	mutable map<string, long> lKeys_;
	mutable map<string, double> dKeys_;

	int internalIndex_;
	GribInterpretor* interpretor_;
	map<double, std::set<double> > positions_;




	grib_handle*  handle_;
	grib_nearest* nearest_;
	grib_handle*  field_;
	grib_handle*  component1_;
	grib_handle*  component2_;
	grib_handle*  colour_;

	string title_;
	static int count_;
	friend class GribInterpretor;
private:
	//! Copy constructor - No copy allowed
	GribDecoder(const GribDecoder&);
	//! Overloaded << operator to copy - No copy allowed
	GribDecoder& operator=(const GribDecoder&);

// -- Friends
	//! Overloaded << operator to call print().
	friend ostream& operator<<(ostream& s,const GribDecoder& p)
		{ p.print(s); return s; }
};


class GribEntryDecoder: public GribDecoder
{
public:
	GribEntryDecoder(grib_handle* handle) {
		handle_ = handle;
		handle1_ = 0;
		handle2_ = 0;
		handle3_ = 0;
		Data::dimension_ = 1;

	}
	GribEntryDecoder(grib_handle* handle1, grib_handle* handle2) {
		handle_ = handle1;
		handle1_ = handle1;
		handle2_ = handle2;
		handle3_ = 0;

		Data::dimension_ = 2;
	}
	GribEntryDecoder(grib_handle* handle1, grib_handle* handle2, grib_handle* handle3) {
		handle_ = handle1;
		handle1_ = handle1;
		handle2_ = handle2;
		handle3_ = handle3;
		Data::dimension_ = 3;
	}
	~GribEntryDecoder() {}

	grib_handle* open(grib_handle*, bool sendMsg = true);

	void openFirstComponent() {
		ASSERT(handle1_);
		handle_ = handle1_;
		component1_ = handle1_;
	}

	void openSecondComponent() {
		ASSERT(handle2_);
		handle_ = handle2_;
		component2_ = handle2_;
	}
	void openThirdComponent() {
			ASSERT(handle3_);
			handle_ = handle3_;
		}
	void readColourComponent() {
		if ( handle3_ ) {
			handle_ = handle3_;
			read(&colourComponent_);
		}
		else
			colourComponent_ = 0;
	}

protected:
	grib_handle*  handle1_;
	grib_handle*  handle2_;
	grib_handle*  handle3_;
};

class GribLoop : public GribLoopAttributes, public DataLoop
{
public:
	GribLoop();
	virtual ~GribLoop();



	void set(const map<string, string>& map) { GribLoopAttributes::set(map); }
	void set(const XmlNode& node) { GribLoopAttributes::set(node); }


	Data* current();
	bool         hasMore();
	void         next();
	void setToFirst();




protected:
	virtual void print(ostream&) const;
	vector<GribDecoder*> gribs_;
    GribDecoder* currentgrib_;
	friend class GribDecoder;
	vector<int>::iterator currentDim_;
	vector<long int>::iterator currentPos_;


	FILE* file_;
	static map<string, string>  ids_;
	static int  index_;
	int uniqueId_;
	int counter_;

private:
	//! Copy constructor - No copy allowed
	GribLoop(const GribLoop&);
	//! Overloaded << operator to copy - No copy allowed
	GribLoop& operator=(const GribLoop&);

// -- Friends
	//! Overloaded << operator to call print().
	friend ostream& operator<<(ostream& s,const GribLoop& p)
		{ p.print(s); return s; }

};




} // namespace magics
#endif
