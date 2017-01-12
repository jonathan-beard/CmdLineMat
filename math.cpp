/**
 * math.cpp - I got tired of writing flash cards over and over,
 * and my son was quite distracted by all the flashy math games.
 * I suspect he was paying far more attention to the graphics than
 * he was the math. I built this and it was  almost an instant hit and
 * he seems to be getting far faster with his arithmetic. I'll eventually
 * clean up the code if he sticks to it. I wrote this in about 10 minutes
 * so it's fairly basic and well....by my standards quite messy, so 
 * please excuse.
 *
 * @author: Jonathan Beard
 * @version: Sun Oct 25 16:58:29 2015
 * 
 * Copyright 2015 Jonathan Beard
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <iomanip>
#include <array>
#include <random>
#include <unistd.h>
#include <sstream>
#include <fstream>
#include <chrono>
#include <functional>
#include <cassert>
/** if you don't have this one, grab CmdArgs from my repo **/
#include <cmd>
#include <signal.h>
#include <unistd.h>
#include <iomanip>
#include <thread>

/** unfortunate, but sig handler so...**/
static std::ofstream userlog;
static double fraction_correct = 0.0;
decltype( std::chrono::system_clock::now() ) start;

static
void sig_handler( int sig )
{
   if( userlog.is_open() )
   {
      userlog << std::flush;
      userlog.close();
   }
   std::cout << "\n\n";
   std::cout << "Your % correct is: " << std::setprecision( 5 ) << 
      fraction_correct << std::endl;
   //just in case we interrupted the say cmd
   unlink( "/tmp/speach" );
   exit( EXIT_SUCCESS );
}

class problem
{
public:
   problem( std::ostream &logstream ) : output_stream( std::cout ),
                                        logstream( logstream )
   {
   }

   virtual ~problem() = default;

   virtual bool run( const std::int64_t a, 
                     const std::int64_t b ) = 0;
   
   virtual bool constrain( const std::int64_t a,
                           const std::int64_t b )
   {
       return( true );
   }
   
protected:
   /** vars **/
   const std::string what = { "What does: " };
   std::ostream      &output_stream;
   std::ostream      &logstream;
   

   /** funcs **/
   static std::string make_prob(
      const std::int64_t a,
      const std::int64_t b,
      const std::string &op )
   {
      return( std::to_string( a ) + 
               " " + op + 
                  " " + std::to_string( b ) );
   }

   static
   void do_speech( const std::string str )
   {
   	std::ofstream ofs( "/tmp/speach" );
   	ofs << str;
   	ofs.close();
   	system( "say -f /tmp/speach" );
   	unlink( "/tmp/speach" );
   	return;
   }
  
   /**
    * check_ans - check the ans against the key. If they match
    * then return true, else false. If the answer is correct
    * then say it's correct, else say try again.
    * @param ans - const T&, answer given by user
    * @param key - const T&, supplied correct answer
    * @return  bool, true if correct
    */
   template < typename T, 
              typename std::enable_if< 
               std::is_fundamental< T >::value >::type* = nullptr >
   bool check_ans( const T &ans, 
                   const T &key ) 
   {
      auto end = std::chrono::system_clock::now();
      std::chrono::duration< double > diff = end-start;
      if( ans  == key )
      {
        std::stringstream ss;
        ss <<  ans  <<  " is correct in " << 
            std::setprecision(2) << diff.count() << 
                " seconds , GOOD JOB!!" << std::endl;	

        output_stream << ss.str() << "\n";
      	do_speech( ss.str() );
        return( true );
      }
      std::stringstream ss;
      ss << ans <<  " is incorrect in " << 
        std::setprecision( 2 ) << diff.count() << 
            " seconds, lets maybe try another" << std::endl;
      output_stream << ss.str() << "\n";
      do_speech( ss.str() );
      return( false );
   }

   /**
    * ask_prob - asks the problem from the user via 
    * output_stream and via speech.
    * @param   prob - const std::string&, problem to ask about
    */
   void ask_prob( const std::string &prob )
   {
      output_stream << what << prob << " = " << std::flush;
   }
}; /** end problem **/

class board
{
public:
   board( const std::int64_t min,
          const std::int64_t max ) : 
            gen( std::chrono::system_clock::now().time_since_epoch().count() ),
                                     min( min ),
                                     max( max )
   {
   }

   virtual ~board()
   {
      for( auto *ptr : probs )
      {
         delete( ptr );
      }
   }

   void add( problem * const p )
   {
      assert( p != nullptr );
      probs.push_back( p );
   }

   void operator ()()
   {
      /** pick random type **/
      problem *p( nullptr );
      if( probs.size() == 1 )
      {
         p = probs[ 0 ]; 
      }
      else
      {
         std::uniform_int_distribution< std::uint32_t  > type_index( 0, probs.size() - 1 );
         p = probs[ type_index( gen ) ];
      }
      assert( p != nullptr );

      /** feed random numbers **/
      std::uniform_int_distribution< std::int64_t  > num_gen( min, max );
      auto num_a( num_gen( gen ) );
	  auto num_b( num_gen( gen ) );
      while( ! p->constrain( num_a, num_b ) )
      {
         num_a = num_gen( gen );
	     num_b = num_gen( gen );
      }
      count++;
      
      auto function([&]( const std::int64_t numA, const std::int64_t numB ) -> void
      {
        if( p->run( numA, numB ) )
        {
           correct++;
        }
        return;
      } );

      start = std::chrono::system_clock::now();
      std::thread th( function, num_a, num_b );
      //FIXME, come back here
      th.join();
   }

   std::uint32_t getCount()
   {
      return( count );
   }

   double getPercentageCorrect()
   {
      if( count == 0 )
      {
         return( 0.0 );
      }
      else
      {
         return( ( (double)correct / (double)count ) * 100.00 );
      }
   }
private:
   std::vector< problem* > probs;
   std::default_random_engine gen;
   const std::int64_t min;
   const std::int64_t max;
   std::uint32_t      correct = 0;
   std::uint32_t      count   = 0;
};

class add : public problem
{
public:
   add( std::ostream &logstream ) : problem( logstream ){}

   virtual bool run( const std::int64_t a,
                     const std::int64_t b )
   {
      const auto prob( make_prob( a, b, "+" ) );
      const auto key( a + b );
      ask_prob( prob );
      do_speech( what + prob + " equal" );
      std::int64_t ans( 0 );
      std::cin >> ans;
      if( check_ans( ans, key ) )
      {
         
         if( logstream.good() )
         {
            logstream << "correct,   " << prob << ", " << ans << std::endl;
         }
         return( true );
      }
      else
      {
         if( logstream.good() )
         {
            logstream << "incorrect, " << prob << ", " << ans << std::endl;
         }
         return( false );
      }
   }
};

class sub : public problem
{
public:
   sub( std::ostream &logstream,
        const bool noneg ) : problem ( logstream ),
                             noneg( noneg ){}

   
   virtual bool constrain( const std::int64_t a,
                           const std::int64_t b )
   {
        if( noneg )
        {
            if( b > a )
            {
                return( false );
            }
        }
        return( true );
   }

   virtual bool run( const std::int64_t a,
                     const std::int64_t b )
   {
      const auto prob( make_prob( a, b, "-" ) );
      const auto prob_spoken( make_prob( a, b, "minus" ) );
      const auto key( a - b );
      ask_prob( prob );
      do_speech( what + prob_spoken + " equal" );
      std::int64_t ans( 0 );
      std::cin >> ans;
      if( check_ans( ans, key ) )
      {
         if( logstream.good() )
         {
            logstream << "correct,   " << prob << ", " << ans << std::endl;
         }
         return( true );
      }
      else
      {
         if( logstream.good() )
         {
            logstream << "incorrect, " << prob << ", " << ans << std::endl;
         }
         return( false );
      }
   }
private:
   const bool noneg;
};


//add op selector via cmd line args
int
main( int argc, char **argv )
{
   
   if( signal( SIGINT, sig_handler ) == SIG_ERR )
   {
      perror( "Failed to set signal handler, exiting!" );
      exit( EXIT_FAILURE );
   }

   bool addition( true );
   bool subtraction( false );
   bool multiplication( false );
   bool help( false );
   
   std::int64_t max(  9 );
   std::int64_t min( -9 );
   std::int64_t num_problems( 20 );
   double frac_to_exit( .75 );
   bool noneg( false );

   std::string logfile( "" );
   

   CmdArgs cmd( argv[ 0 ],
                std::cout,
                std::cerr );

   cmd.addOption( new Option< bool >( addition,
                                      "-add",
                                      "Include addition problems" ) );
   cmd.addOption( new Option< bool >( subtraction,
                                      "-sub",
                                      "Include subtraction problems" ) );
   cmd.addOption( new Option< bool >( multiplication,
                                      "-mult",
                                      "Include multiplication problems" ) );
   cmd.addOption( new Option< std::int64_t >( num_problems,
                                              "-number",
                                              "Set total number of problems" ) );
   cmd.addOption( new Option< double >( frac_to_exit,
                                              "-frac_success",
                                              "Set fraction .xx to get correct before exiting" ) );
   cmd.addOption( new Option< std::int64_t >( min,
                                              "-min",
                                              "Set minimum digit to use" ) );
   cmd.addOption( new Option< std::int64_t >( max,
                                              "-max",
                                              "Set maximum digit to use" ) );
   cmd.addOption( new Option< std::string >( logfile,
                                             "-log",
                                             "Set a log file, .csv extension added by default" ) );
   cmd.addOption( new Option< bool >( help,
                                      "-h",
                                      "Print menu and exit" ) );

   cmd.addOption( new Option< bool >( noneg,
                                      "-noneg",
                                      "prevent the user from getting negative numbers" ) );

   cmd.processArgs( argc, argv );
   if( help )
   {
      cmd.printArgs();
      std::exit( EXIT_SUCCESS );
   }
   
   /** open logfile **/
   if( logfile.length() > 0 )
   {
      /** log is global **/
      userlog.open( logfile + ".csv" ); 
   }
 
   board blackboard( min, max );
   
   if( addition )
   {
      blackboard.add( new add( userlog ) );
   }
   if( subtraction )
   {
      blackboard.add( new sub( userlog, noneg ) );
   }

	
   while( blackboard.getCount() < num_problems  || fraction_correct < frac_to_exit )
   {
      blackboard();
	  fraction_correct = blackboard.getPercentageCorrect();
   }

   if( userlog.is_open() )
   {
      userlog.close();
   }
	return( EXIT_SUCCESS );
}
