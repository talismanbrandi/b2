//This file is part of Bertini 2.
//
//powerseries_endgame.hpp is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
//powerseries_endgame.hpp is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with powerseries_endgame.hpp.  If not, see <http://www.gnu.org/licenses/>.
//
// Copyright(C) 2015, 2016 by Bertini2 Development Team
//
// See <http://www.gnu.org/licenses/> for a copy of the license, 
// as well as COPYING.  Bertini2 is provided with permitted 
// additional terms in the b2/licenses/ directory.

// individual authors of this file include:
// daniel brake, university of notre dame
// Tim Hodges, Colorado State University


#pragma once

#include "bertini2/tracking/base_endgame.hpp"


namespace bertini{ namespace tracking { namespace endgame{

inline
dbl operator*(unsigned i, dbl z)
{
	z*=i;
	return z;
}
/** 
\class PowerSeriesEndgame

\brief class used to finish tracking paths during Homotopy Continuation


## Explanation

The bertini::PowerSeriesEndgame class enables us to finish tracking on possibly singular paths on an arbitrary square homotopy.  

The intended usage is to:

1. Create a system, tracker, and instantiate some settings.
2. Using the tracker created track to the engame boundary, by default this is t = 0.1. 
3. Create a PowerSeriesEndgame, associating it to the tracker you wish to use. The tracker knows the system being solved.
4. For each path being tracked send the PowerSeriesEndgame the time value and other variable values that it should use to start the endgame. 
5. The PowerSeriesEndgame, if successful, will store the homotopy solutions at t = 0.

## Example Usage

Below we demonstrate a basic usage of the PowerSeriesEndgame class to find the singularity at t = 0. 

The pattern is as described above: create an instance of the class, feeding it the system to be used, and the endgame boundary time and other variable values at the endgame boundary. 

\code{.cpp}
using namespace bertini::tracking;
using RealT = TrackerTraits<TrackerType>::BaseRealType; // Real types
using ComplexT = TrackerTraits<TrackerType>::BaseComplexType; Complex types

// 1. Define the polynomial system that we wish to solve. 
System target_sys;
Var x = std::make_shared<Variable>("x"), t = std::make_shared<Variable>("t"), y = std::make_shared<Variable>("y");

VariableGroup vars{x,y};
target_sys.AddVariableGroup(vars); 

target_sys.AddFunction((pow(x-1,3));
target_sys.AddFunction((pow(y-1,2));

// 1b. Homogenize and patch the polynomial system to work over projective space. 
sys.Homogenize();
sys.AutoPatch();

// 2. Create a start system, for us we will use a total degree start system.
auto TD_start_sys = bertini::start_system::TotalDegree(target_sys);

// 2b. Creating homotopy between the start system and system we wish to solve. 
auto my_homotopy = (1-t)*target_sys + t*TD_start_sys*Rational::Rand(); //the random number is our gamma for a random path between t = 1 and t = 0.
my_homotopy.AddPathVariable(t);

//Sets up configuration settings for our particular system.
auto precision_config = PrecisionConfig(my_homotopy);


// 3. Creating a tracker. For us this is an AMPTracker. 
AMPTracker tracker(my_homotopy);

//Tracker setup of settings. 
config::Stepping<RealT> stepping_preferences;
stepping_preferences.initial_step_size = RealT(1)/RealT(5);// change a stepping preference
config::Newton newton_preferences;
tracker.Setup(TestedPredictor,
            RealFromString("1e-6"),
            RealFromString("1e5"),
        stepping_preferences,
        newton_preferences);
tracker.PrecisionSetup(precision_config);

//We start at t = 1, and will stop at t = 0.1 before starting the endgames. 
ComplexT t_start(1), t_endgame_boundary(0.1);

//This will hold our solutions at t = 0.1 
std::vector<Vec<ComplexT> > my_homotopy_solutions_at_endgame_boundary;

// result holds the value we track to at 0.1, and tracking success will report if we are unsucessful.
Vec<ComplexT> result;

//4. Track all points to 0.1
for (unsigned ii = 0; ii < TD_start_sys.NumStartPoints(); ++ii)
{
    mpfr_float::default_precision(ambient_precision);
    my_homotopy.precision(ambient_precision); // making sure our precision is all set up 
    auto start_point = TD_start_sys.StartPoint<ComplexT>(ii);

    tracker.TrackPath(result,t_start,t_endgame_boundary,start_point);

    my_homotopy_solutions_at_endgame_boundary.push_back(result);
}


//Settings for the endgames. 

config::Tolerances<RealT> tolerances;
tolerances.final_tolerance_multiplier = RealT(100);

config::PowerSeries power_series_settings;
power_series_settings.max_cycle_number = 4;


// 5. Create a power series endgame, and use them to get the soutions at t = 0. 
EndgameSelector<TrackerType>::PSEG my_pseg_endgame(tracker,power_series_settings,tolerances);


std::vector<Vec<ComplexT> > my_homotopy_solutions; 

std::vector<Vec<ComplexT> > my_homotopy_divergent_paths; 

for(auto s : my_homotopy_solutions_at_endgame_boundary) 
{
    SuccessCode endgame_success = my_pseg_endgame.Run(t_endgame_boundary,s);

    if(endgame_success == SuccessCode::Success)
    {
        my_homotopy_solutions.push_back(my_homotopy.DehomogenizePoint(my_endgame.FinalApproximation<ComplexT>()));
    }
    else
    {
        my_homotopy_divergent_paths.push_back(my_homotopy.DehomogenizePoint(my_endgame.FinalApproximation<ComplexT>()));
    }
}


\endcode

If this documentation is insufficient, please contact the authors with suggestions, or get involved!  Pull requests welcomed.

## Testing

Test suite driving this class: endgames_test.

File: test/endgames/generic_pseg_test.hpp
File: test/endgames/amp_powerseries_test.cpp
File: test/endgames/fixed_double_powerseries_test.cpp
FIle: test/endgames/fixed_multiple_powerseries_test.cpp
*/

template<typename TrackerType, typename FinalPSEG, typename... UsedNumTs> 
class PowerSeriesEndgame : public EndgameBase<TrackerType, FinalPSEG>
{

	// convert the base endgame into the derived type.
	const FinalPSEG& AsDerived() const
	{
		return static_cast<const FinalPSEG&>(*this);
	}

protected:

	using BaseComplexType = typename TrackerTraits<TrackerType>::BaseComplexType;
	using BaseRealType = typename TrackerTraits<TrackerType>::BaseRealType;

	using BCT = BaseComplexType;
	using BRT = BaseRealType;
				
	/**
	\brief State variable representing a computed upper bound on the cycle number.
	*/
	mutable unsigned upper_bound_on_cycle_number_;

	/**
	\brief Settings that are specific to the Power series endgame. 
	*/	
	config::PowerSeries power_series_settings_; 

	/**
	\brief Holds the time values for different space values used in the Power series endgame. 
	*/	
	mutable std::tuple< TimeCont<UsedNumTs>... > times_;

	/**
	\brief Holds the space values used in the Power series endgame. 
	*/			
	mutable std::tuple< SampCont<UsedNumTs>... > samples_;

	/**
	\brief Holds the derivatives at each space point. 
	*/			
	mutable std::tuple< SampCont<UsedNumTs>... > derivatives_;

	/**
	\brief Random vector used in computing an upper bound on the cycle number. 
	*/
	mutable Vec<BCT> rand_vector;

public:

	auto UpperBoundOnCycleNumber() const { return upper_bound_on_cycle_number_;}

	const config::PowerSeries& PowerSeriesSettings() const
	{
		return power_series_settings_;
	}

	/**
	\brief Function that clears all samples and times from data members for the Power Series endgame
	*/	
	template<typename CT>
	void ClearTimesAndSamples()
	{
		std::get<TimeCont<CT> >(times_).clear(); 
		std::get<SampCont<CT> >(samples_).clear();
	}

	/**
	\brief Function to set the times used for the Power Series endgame.
	*/	
	template<typename CT>
	void SetTimes(TimeCont<CT> times_to_set) { std::get<TimeCont<CT> >(times_) = times_to_set;}

	/**
	\brief Function to get the times used for the Power Series endgame.
	*/	
	template<typename CT>
	auto GetTimes() const {return std::get<TimeCont<CT> >(times_);}

	/**
	\brief Function to set the space values used for the Power Series endgame.
	*/	
	template<typename CT>
	void SetSamples(SampCont<CT> samples_to_set) { std::get<SampCont<CT> >(samples_) = samples_to_set;}

	/**
	\brief Function to get the space values used for the Power Series endgame.
	*/	
	template<typename CT>
	auto GetSamples() const {return std::get<SampCont<CT> >(samples_);}

	/**
	\brief Function to set the times used for the Power Series endgame.
	// */	
	template<typename CT>
	void SetRandVec(Vec<CT> sample) {rand_vector = Vec<CT>::Random(sample.size());}



	/**
	\brief Setter for the specific settings in tracking_conifg.hpp under PowerSeries.
	*/	
	void SetPowerSeriesSettings(config::PowerSeries new_power_series_settings){power_series_settings_ = new_power_series_settings;}


	explicit PowerSeriesEndgame(TrackerType const& tr, 
	                            const std::tuple< config::PowerSeries const&, 
            					const config::Endgame<BRT>&, 
            					const config::Security<BRT>&, 
            					const config::Tolerances<BRT>& >& settings )
      : EndgameBase<TrackerType, FinalPSEG>(tr, std::get<1>(settings), std::get<2>(settings), std::get<3>(settings) ), 
          power_series_settings_( std::get<0>(settings) )
   	{}

    template< typename... Ts >
		PowerSeriesEndgame(TrackerType const& tr, const Ts&... ts ) : PowerSeriesEndgame(tr, Unpermute<config::PowerSeries, config::Endgame<BRT>, config::Security<BRT>, config::Tolerances<BRT> >( ts... ) ) 
		{}


	~PowerSeriesEndgame() {};


	/**
	\brief Computes an upper bound on the cycle number. Consult page 53 of \cite bertinibook.

	## Input: 
			None: all data needed are class data members.

	## Output:
			upper_bound_on_cycle_number_: Used for an exhaustive search for the best cycle number for approimating the path to t = 0.

	##Details:
			\tparam CT The complex number type.
	*/
	template<typename CT>
	unsigned ComputeBoundOnCycleNumber()
	{ 
		using RT = typename Eigen::NumTraits<CT>::Real;
		using std::log; using std::abs;

		const auto& samples = std::get<SampCont<CT> >(samples_);
		assert(samples.size()>=3 && "must have at least three sample points to estimate the cycle number");
		auto num_samples = samples.size();
		const Vec<CT> & sample0 = samples[num_samples-3];
		const Vec<CT> & sample1 = samples[num_samples-2];
		const Vec<CT> & sample2 = samples[num_samples-1]; // most recent sample.  oldest samples at front of the container

		// Vec<BCT> rand_vector = Vec<CT>::Random(sample0.size()); // todo: eliminate this temporary vector, make permanent.
		
		if (sample2==sample1 || sample1==sample0)
		{
			upper_bound_on_cycle_number_ = 1;
			return upper_bound_on_cycle_number_;
		}

		//DO NOT USE Eigen .dot() it will do conjugate transpose which is not what we want.
		RT estimate = abs(log(this->EndgameSettings().sample_factor))
						/
					  abs(
		                  log(
		                      abs(
		                          ((sample2 - sample1).transpose()*rand_vector).norm()
		                          /
		                          ((sample1 - sample0).transpose()*rand_vector).norm()
		                          )
		                      )
		                  );

		if (estimate < 1) // would be nan if sample points are same as each other
		  	upper_bound_on_cycle_number_ = 1;
		else
		{
			using std::max;
			//casting issues between auto and unsigned integer. TODO: Try to stream line this.
			unsigned int upper_bound;
			RT upper_bound_before_casting = round(floor(estimate + RT(.5))*power_series_settings_.cycle_number_amplification);
			upper_bound = unsigned (upper_bound_before_casting);
			upper_bound_on_cycle_number_ = max(upper_bound,power_series_settings_.max_cycle_number);
		}

		return upper_bound_on_cycle_number_;
	}//end ComputeBoundOnCycleNumber




	/**
	\brief This function computes the cycle number using an exhaustive search up the upper bound computed by the above function BoundOnCyleNumber. 

		## Input: 
				None: all data needed are class data members.

		## Output:
				cycle_number_: Used to create a hermite interpolation to t = 0. 

		##Details:
				\tparam CT The complex number type.
			This is done by an exhaustive search from 1 to upper_bound_on_cycle_number. There is a conversion to the s-space from t-space in this function. 
	As a by-product the derivatives at each of the samples is returned for further use. 
	*/

	template<typename CT>
	unsigned ComputeCycleNumber()
	{
		using RT = typename Eigen::NumTraits<CT>::Real;

		//Compute upper bound for cycle number.
		ComputeBoundOnCycleNumber<CT>();


		SampCont<CT> samples = std::get<SampCont<CT> >(samples_); // take a copy...  replace this asap with something that doesn't copy the data...
			const auto& times   = std::get<TimeCont<CT> >(times_);
			const auto& derivatives = std::get<SampCont<CT> >(derivatives_);

		assert((samples.size() == times.size()) && "must have same number of times and samples");

		if (derivatives.empty())
			ComputeDerivatives<CT>();
		else
			assert((samples.size() == derivatives.size()) && "must have same number of samples and derivatives");

		assert((samples.size() >= this->EndgameSettings().num_sample_points) && "must have sufficiently many sample points");

		
		const Vec<CT> most_recent_sample = samples.back();  // take a copy of the vector
		samples.pop_back();// again, this should be replaced by something that doesn't need a copy of the samples
		const CT most_recent_time = times.back();

		//Now we actually compute the Cycle Number

		//num_used_points is (num_sample_points-1)
		//because we are using the most current sample to do an 
		//exhaustive search for the best cycle number. 
		//if there are less samples than num_sample_points return samples.size() otherwise return num_sample_points.
		

		unsigned num_used_points = samples.size() < this->EndgameSettings().num_sample_points 
									?
								   samples.size() : this->EndgameSettings().num_sample_points ;

		unsigned offset = samples.size() - num_used_points;
		auto min_found_difference = Eigen::NumTraits<RT>::highest();

		TimeCont<CT> s_times(num_used_points);
		SampCont<CT> s_derivatives(num_used_points);


		for(unsigned int candidate = 1; candidate <= upper_bound_on_cycle_number_; ++candidate)
		{			
			BOOST_LOG_TRIVIAL(severity_level::trace) << "testing cycle candidate " << candidate;

			for(unsigned int ii=0; ii<num_used_points; ++ii)// using the last sample to predict to. 
			{   using std::pow;
				s_times[ii] = pow(times[ii+offset],1/static_cast<RT>(candidate));
				s_derivatives[ii] = derivatives[ii+offset] * (candidate * pow(times[ii+offset], static_cast<RT>(candidate-1)/candidate));
			}

			RT curr_diff = (HermiteInterpolateAndSolve(
			                      pow(most_recent_time,static_cast<RT>(1)/candidate), // the target time
			                      num_used_points,s_times,samples,s_derivatives) // the input data
			                 - 
			                 most_recent_sample).norm();

			if (curr_diff < min_found_difference)
			{
				min_found_difference = curr_diff;
				this->cycle_number_ = candidate;
			}

		}// end cc loop over cycle number possibilities
		BOOST_LOG_TRIVIAL(severity_level::trace) << "cycle number computed to be " << this->CycleNumber();

		return this->cycle_number_;
	}//end ComputeCycleNumber


	/**
		\brief Compute a set of derivatives using internal data to the endgame.

		## Input: 
				None: all data needed are class data members.

		## Output:
				None: Derivatives are members of this class.

		##Details:
				\tparam CT The complex number type.
	*/
	template<typename CT>
	void ComputeDerivatives()
	{
		auto& samples = std::get<SampCont<CT> >(samples_);
		auto& times   = std::get<TimeCont<CT> >(times_);
		auto& derivatives = std::get<SampCont<CT> >(derivatives_);

		assert((samples.size() == times.size()) && "must have same number of times and samples");

		if (TrackerTraits<TrackerType>::IsAdaptivePrec) // known at compile time
		{
			auto max_precision = AsDerived().EnsureAtUniformPrecision(times, samples);
			this->GetSystem().precision(max_precision);
		}

		//Compute dx_dt for each sample.
		derivatives.clear(); derivatives.resize(samples.size());
		for(unsigned ii = 0; ii < samples.size(); ++ii)
		{	
			// uses LU look at Eigen documentation on inverse in Eigen/LU.
		 	derivatives[ii] = -(this->GetSystem().Jacobian(samples[ii],times[ii]).inverse())*(this->GetSystem().TimeDerivative(samples[ii],times[ii]));
		}
	}
	/**
	\brief This function computes an approximation of the space value at the time time_t0. 

		## Input: 
				result: Passed by reference this holds the value of the approximation we compute
				t0: This is the time value for which we wish to compute an approximation at. 

		## Output:
				SuccessCode: This reports back if we were successful in making an approximation.

		##Details:
	\tparam CT The complex number type.
				This function handles computing an approximation at the origin. 
				We compute the cycle number best for the approximation, and convert derivatives and times to the s-plane where s = t^(1/c).
				We use the converted times and derivatives along with the samples to do a Hermite interpolation.
	*/
	template<typename CT>
	SuccessCode ComputeApproximationOfXAtT0(Vec<CT>& result, const CT & t0)
	{	
		using RT = typename Eigen::NumTraits<CT>::Real;

		const auto& samples = std::get<SampCont<CT> >(samples_);
		const auto& times   = std::get<TimeCont<CT> >(times_);
		const auto& derivatives  = std::get<SampCont<CT> >(derivatives_);

		auto num_sample_points = this->EndgameSettings().num_sample_points;

		assert(samples.size()==times.size() && "must have same number of samples in times and spaces");

		if (derivatives.empty())
			ComputeDerivatives<CT>();
		else
			assert((samples.size() == derivatives.size()) && "must have same number of samples and derivatives");

			assert(samples.size()>=num_sample_points && "must have sufficient number of samples");
			assert(times.size()>=num_sample_points && "must have sufficient number of times");
			assert(derivatives.size()>=num_sample_points && "must have sufficient number of derivatives");

			ComputeCycleNumber<CT>();
		auto c = this->CycleNumber();
		// Conversion to S-plane.

		
		auto offset = samples.size() - num_sample_points;

		TimeCont<CT> s_times(num_sample_points);
		SampCont<CT> s_derivatives(num_sample_points);

		for(unsigned ii = 0; ii < num_sample_points; ++ii){
			if (c==0)
				throw std::runtime_error("cycle number is 0 while computing approximation of root at target time");

			s_times[ii] = pow(times[ii+offset],static_cast<RT>(1)/c);
			s_derivatives[ii] = derivatives[ii+offset]*( c*pow(times[ii+offset],static_cast<RT>(c-1)/c ));
		}

		result = HermiteInterpolateAndSolve(pow(t0,static_cast<RT>(1)/c), num_sample_points, s_times, samples, s_derivatives);
		return SuccessCode::Success;
	}//end ComputeApproximationOfXAtT0



	/**
		\brief The samples used in the power series endgame are collected by advancing time to t = 0, by multiplying the current time by the sample factor. 

		## Input: 
				None: all data needed are class data members.

		## Output:
				SuccessCode: This reports back if we were successful in advancing time. 

		##Details:
				\tparam CT The complex number type.
				This function computes the next time value for the power series endgame. After computing this time value, 
				it will track to it and compute the derivative at this time value for further appoximations to be made during the
				endgame.
	*/
	template<typename CT>
	SuccessCode AdvanceTime()
	{
		auto& samples = std::get<SampCont<CT> >(samples_);
		auto& times   = std::get<TimeCont<CT> >(times_);
		auto& derivatives  = std::get<SampCont<CT> >(derivatives_);

		Vec<CT> next_sample;
		CT next_time = times.back() * this->EndgameSettings().sample_factor; //setting up next time value.

  		if (abs(next_time) < this->EndgameSettings().min_track_time)
  		{
  			BOOST_LOG_TRIVIAL(severity_level::trace) << "Current time norm is less than min track time." << '\n';

  			return SuccessCode::MinTrackTimeReached;
  		}


  		BOOST_LOG_TRIVIAL(severity_level::trace) << "tracking to t = " << next_time << ", default precision: " << DefaultPrecision() << "\n";
		SuccessCode tracking_success = this->GetTracker().TrackPath(next_sample,times.back(),next_time,samples.back());
		if (tracking_success != SuccessCode::Success)
			return tracking_success;

		AsDerived().EnsureAtPrecision(next_time,Precision(next_sample));
		
		times.push_back(next_time);
		samples.push_back(next_sample);

		auto refine_success = AsDerived().RefineSample(samples.back(), next_sample,  times.back());
		if (refine_success != SuccessCode::Success)
		{
			BOOST_LOG_TRIVIAL(severity_level::trace) << "refining failed, code " << int(refine_success);
			return refine_success;
		}


 		auto max_precision = AsDerived().EnsureAtUniformPrecision(times, samples, derivatives);
		this->GetSystem().precision(max_precision);

		derivatives.push_back(-(this->GetSystem().Jacobian(samples.back(),times.back()).inverse())*(this->GetSystem().TimeDerivative(samples.back(),times.back())));

 		return SuccessCode::Success;
	}


	/**
	\brief Primary function running the Power Series endgame. 

		## Input: 
				start_time: This is the time value for which the endgame begins, by default this is t = 0.1
				start_point: An approximate solution of the homotopy at t = start_time

		## Output:
				SuccessCode: This reports back if we were successful in advancing time. 

		##Details:
	\tparam CT The complex number type.
				Tracking forward with the number of sample points, this function will make approximations using Hermite interpolation. This process will continue until two consecutive
				approximations are withing final tolerance of each other. 
	*/		
	template<typename CT>
	SuccessCode Run(const CT & start_time, const Vec<CT> & start_point)
	{
		if (start_point.size()!=this->GetSystem().NumVariables())
		{
			std::stringstream err_msg;
			err_msg << "number of variables in start point for PSEG, " << start_point.size() << ", must match the number of variables in the system, " << this->GetSystem().NumVariables();
			throw std::runtime_error(err_msg.str());
		}

		BOOST_LOG_TRIVIAL(severity_level::trace) << "\n\nPSEG(), default precision: " << DefaultPrecision() << "\n\n";
		BOOST_LOG_TRIVIAL(severity_level::trace) << "start point precision: " << Precision(start_point(0)) << "\n\n";

		DefaultPrecision(Precision(start_point(0)));

		using RT = typename Eigen::NumTraits<CT>::Real;
		//Set up for the endgame.
			ClearTimesAndSamples<CT>();

			auto& samples = std::get<SampCont<CT> >(samples_);
			auto& times   = std::get<TimeCont<CT> >(times_);
			auto& derivatives  = std::get<SampCont<CT> >(derivatives_);
			Vec<CT>& final_approx = std::get<Vec<CT> >(this->final_approximation_at_origin_);
			SetRandVec(start_point);

	 	RT approx_error(1);  //setting up the error of successive approximations. 
	 	
	 	CT origin(0);

		auto initial_sample_success = this->ComputeInitialSamples(start_time, start_point, times, samples);

		if (initial_sample_success!=SuccessCode::Success)
		{
			BOOST_LOG_TRIVIAL(severity_level::trace) << "initial sample gathering failed, code " << int(initial_sample_success) << std::endl;
			return initial_sample_success;
		}

		ComputeDerivatives<CT>();

	 	Vec<CT> prev_approx;
	 	auto extrapolation_code = ComputeApproximationOfXAtT0(prev_approx, origin);
	 	final_approx = prev_approx;

	 	if (extrapolation_code != SuccessCode::Success)
	 		return extrapolation_code;


	 	RT norm_of_dehom_of_prev_approx;
	 	if (this->SecuritySettings().level <= 0)
	 	 	norm_of_dehom_of_prev_approx = this->GetSystem().DehomogenizePoint(prev_approx).norm();

	 	

	  	Vec<CT> latest_approx;
	    RT norm_of_dehom_of_latest_approx;


		while (approx_error > this->Tolerances().final_tolerance)
		{
	  		auto advance_code = AdvanceTime<CT>();
	  		if (advance_code!=SuccessCode::Success)
	 		{
	 			BOOST_LOG_TRIVIAL(severity_level::trace) << "unable to advance time, code " << int(advance_code);
	 			return advance_code;
	 		}

	 		extrapolation_code = ComputeApproximationOfXAtT0(latest_approx, origin);
	 		if (extrapolation_code!=SuccessCode::Success)
	 		{
	 			BOOST_LOG_TRIVIAL(severity_level::trace) << "failed to compute the approximation at " << origin << "\n\n";
	 			return extrapolation_code;
	 		}
	 		BOOST_LOG_TRIVIAL(severity_level::trace) << "latest approximation:\n" << latest_approx << '\n';

	 		if(this->SecuritySettings().level <= 0)
	 		{
	 			norm_of_dehom_of_latest_approx = this->GetSystem().DehomogenizePoint(latest_approx).norm();
		 		if(norm_of_dehom_of_latest_approx > this->SecuritySettings().max_norm && norm_of_dehom_of_prev_approx > this->SecuritySettings().max_norm)
	 				return SuccessCode::SecurityMaxNormReached;
	 		}

	 		approx_error = (latest_approx - prev_approx).norm();
	 		BOOST_LOG_TRIVIAL(severity_level::trace) << "consecutive approximation error:\n" << approx_error << '\n';

	 		prev_approx = latest_approx;
	 		if(this->SecuritySettings().level <= 0)
			    norm_of_dehom_of_prev_approx = norm_of_dehom_of_latest_approx;
		} //end while	
		// in case if we get out of the for loop without setting. 
		final_approx = latest_approx;
		return SuccessCode::Success;

	} //end PSEG

}; // end powerseries class




}}} // re: namespaces
