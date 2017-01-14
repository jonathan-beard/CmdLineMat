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
#include <cstdio>
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
#include <limits>
#include <float.h>

/** constants **/
const static  std::string BLUE          = "\033[1;36m";
const static  std::string NORMAL        = "\033[0m";
const static  std::string RED           = "\033[1;31m";
const static  std::string LIGHT_GREY    = "\033[0;37m";
const static  std::string PURPLE        = "\033[1;35m";

/** unfortunate, but sig handler so...**/
static std::ofstream    userlog;

struct STAT{
    double           fraction         = 0.0;
    double           min_time         = DBL_MAX;
    double           max_time         = -DBL_MAX;
    double           avg_time         = 0.0;
    std::uint32_t    count            = 0;
    double           tot_time         = 0.0;
    static std::uint32_t total;
} correct, incorrect;

std::uint32_t STAT::total = 0;

decltype( std::chrono::system_clock::now() ) start;

static void updateStats( STAT &st, const double curr_dur ) noexcept
{
    st.count++;
    st.tot_time += curr_dur;
    if( st.min_time > curr_dur ){ st.min_time = curr_dur; }
    if( st.max_time < curr_dur ){ st.max_time = curr_dur; }
    st.avg_time = st.tot_time / st.count;
    STAT::total++;
    st.fraction =(double) st.count / (double) STAT::total;
    return;
}

static void printStats( const std::string &&type, const STAT &st, std::ostream &stream )
{
    stream << PURPLE << type << " stats:" << NORMAL << "\n";
    if( st.count == 0 ){ stream << RED << "none" << NORMAL << "\n"; }
    else
    {
    stream << "percentage: " <<  std::setprecision( 4 ) << (st.fraction*100) << "%\n";
    stream << "min time: " << std::setprecision( 2 ) << st.min_time << "s\n";
    stream << "avg time: " << std::setprecision( 2 ) << st.avg_time << "s\n";
    stream << "max time: " << std::setprecision( 2 ) << st.max_time << "s\n\n";
    }
}


static
void sig_handler( int sig )
{
   if( userlog.is_open() )
   {
      userlog << std::flush;
      userlog.close();
   }
   std::cout << "\n\n";
   printStats( "correct", correct, std::cout );
   printStats( "incorrect", incorrect, std::cout );
   //just in case we interrupted the say cmd
   unlink( "/tmp/speach" );
   exit( EXIT_SUCCESS );
}


char* getInput( char *buffer, const std::size_t length, std::size_t &ret )
{
    bool invalid = false;
    std::size_t curr_pos = 0;
    while( curr_pos < length )
    {
        buffer[ curr_pos ] = getc( stdin );
        if( buffer[ curr_pos ] == '\n' )
        {
            ret = curr_pos;
            break;
        }
        else if( ( buffer[ curr_pos ] <  0x30 || buffer[ curr_pos ] > 0x39 ) && buffer[ curr_pos ] != 0x2d )
        {
            /** invalid input **/
            invalid = true;
        }
        curr_pos++;
    }
    if( invalid ){ return( nullptr ); }
    return( buffer );
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
   std::string
   fix_up_neg( const std::string &str )
   {
        std::stringstream ss;
        bool prior_char( false );
        for( auto i( 0 ); i < str.length(); i++ )
        {
            if( str[ i ] == '-' && i == 0 )
            {
                ss << "negative ";
            }
            else if( str[ i ] == '+' || str[ i ] == ':'  )
            {
                prior_char = true;
                ss << str[ i ];
            }
            else if( str[ i ] == '-' && i != 0 )
            {
                if( prior_char )
                {
                    ss << " negative ";
                    prior_char = false;
                }
                else
                {
                    ss << str[ i ];
                }
            }
            else
            {
                ss << str[ i ];
            }
        }
        return( ss.str() );
   }

   static
   void do_speech( const std::string str )
   {
   	std::ofstream ofs( "/tmp/speach" );
   	ofs << fix_up_neg( str );
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
   bool check_ans( const std::string &prob,
                   const T &ans, 
                   const T &key,
                   double  &time,
                   std::ostream &logstream ) 
   {
      auto end = std::chrono::system_clock::now();
      std::chrono::duration< double > diff = end-start;
      time = diff.count();
      if( ans  == key )
      {
        std::stringstream ss;
        ss << BLUE <<  ans  <<  " is correct " << NORMAL << "in " << 
            std::setprecision(2) << time << 
                " seconds, " << PURPLE << "GOOD JOB!!" << NORMAL << std::endl;	

        output_stream << ss.str() << "\n";
        
        std::stringstream speach_ss;
        speach_ss <<  ans  <<  " is correct "  << "in " << 
            std::setprecision( 2 ) << time << 
                " seconds, " << "GOOD JOB!!" << std::endl;	
      	updateStats( correct, time);
        logstream << BLUE << "correct,  " << prob << ", " << ans << ", " << time << std::endl;

        do_speech( speach_ss.str() );
        return( true );
      }

      std::stringstream ss;
      ss << RED << ans <<  " is incorrect " << NORMAL << "in " << 
        std::setprecision( 2 ) << time << 
            " seconds, lets try another" << std::endl;
      output_stream << ss.str() << "\n";
      
      std::stringstream speach_ss;
      speach_ss << ans <<  " is incorrect " << "in " << 
        std::setprecision( 2 ) << time << 
            " seconds, lets try another" << std::endl;
      updateStats( incorrect, time );
      
      logstream << BLUE << "incorrect,  " << prob << ", " << ans << ", " << time << std::endl;
      do_speech( speach_ss.str() );
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
      
      auto function([&p]( const std::int64_t numA, const std::int64_t numB ) -> void
      {
         p->run( numA, numB );
        return;
      } );

      start = std::chrono::system_clock::now();
      std::thread th( function, num_a, num_b );
      
      th.join();
   }


private:
   std::vector< problem* > probs;
   std::default_random_engine gen;
   const std::int64_t min;
   const std::int64_t max;
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
      char buffer[ 128 ];
      std::memset( buffer, '\0', 128 );
      std::size_t ret_count( 0 );
      char *text_ans( nullptr );
      while( ret_count == 0 )
      {
         text_ans = getInput( buffer, 128, ret_count );
         if( text_ans == nullptr && ret_count != 0 ){ std::cout << "invalid input detected, please only numbers\n"; ret_count = 0;}
         /** else spin for answer **/
      }
      ans = strtoll( text_ans, nullptr, 10);
      double time( 0.0 );
      return( check_ans( prob, ans, key, time, logstream ) );
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
      double time( 0.0 );
      std::int64_t ans( 0 );
      char buffer[ 128 ];
      std::memset( buffer, '\0', 128 );
      std::size_t ret_count( 0 );
      char *text_ans( nullptr );
      while( ret_count == 0 )
      {
         text_ans = getInput( buffer, 128, ret_count );
         if( text_ans == nullptr && ret_count != 0 ){ std::cout << "invalid input detected, please only numbers\n"; ret_count = 0;}
         /** else spin for answer **/
      }
      ans = strtoll( text_ans, nullptr, 10);
      return( check_ans( prob, ans, key, time, logstream ) );
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
   if( signal( SIGQUIT, sig_handler ) == SIG_ERR )
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

	
   while( (correct.count + incorrect.count) < num_problems  || 
           correct.fraction < frac_to_exit )
   {
      blackboard();
   }

   if( userlog.is_open() )
   {
      userlog.close();
   }
   printStats( "correct", correct, std::cout );
   printStats( "incorrect", incorrect, std::cout );
   return( EXIT_SUCCESS );
}
