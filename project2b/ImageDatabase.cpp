#include <fstream>
#include "ImageDatabase.h"
#include <sstream>

// Create a database.
ImageDatabase::ImageDatabase()
{
}

// Load a database from file.  The database file contains a list of
// image file names and feature file names.  Each image file name is
// followed by the corresponding feature file name.  The file names in
// the database must be relative to the database path or it won't work.
// I apologize for this annoyance.
bool ImageDatabase::load ( const char * name, bool sift )
{
  DatabaseItem d;

  // Clear all entries from the database.
  clear ();

  // Open the file.
  std::cerr << name << std::endl;
  ifstream f ( name );
  std::string nameString ( name );
  size_t last_slash = nameString.find_last_of ( '/' );
  std::string dir = nameString.substr ( 0, last_slash + 1 );
  std::cerr << dir << std::endl;

  if ( !f.is_open () )
  {
    return false;
  }
  std::cerr << "loaded" << std::endl;

  while ( f )
  {
    std::stringstream ss1, ss2;
    std::string s;
    // Read the image name.
    if ( !( f >> s ) )
    {
      break;
    }
    ss1 << dir;
    ss1 << s;
    d.name = ss1.str ();
    // Read the feature file name.
    ss2 << dir;
    if ( !( f >> s ) )
    {
      break;
    }
    ss2 << s;
    // Load the features from file.
    if ( ( ( !sift ) &&
           ( !d.features.load ( ss2.str ().c_str () ) ) ) ||
         ( ( sift ) && ( !d.features.load_sift ( ss2.str ().c_str () ) ) ) )
    {
      clear ();
      f.close ();
      return false;
    }

    push_back ( d );
  }

  f.close ();
  return true;
}
