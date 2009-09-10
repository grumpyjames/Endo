#include <string>
#include <assert.h>
#include <iostream>

using std::string;

namespace {

  class finish_exception {
  public:
    finish_exception(const string & reason);
    string const & why() { return reason_;}
  private:
    string const reason_;
  };
  
  finish_exception::finish_exception(const string & reason) : reason_(reason) {}
}

void shift_dna_left(string & dna, unsigned int const distance, string & rescue)
{
  int const resulting_length = dna.length() - distance;
  if (resulting_length >= 0) {
    rescue.append(dna.substr(0,distance));
    dna = dna.substr(distance,dna.length()-distance);
  } 
  else {
    dna = "";
  }
}

void push_dna_to_rna(string & dna, string & rna)
{
  if (dna.length() > 6) {
    rna.append(dna.substr(0,7));
    string care;
    shift_dna_left(dna, 7, care);
  }
  else {
    throw finish_exception("Not enough DNA to make RNA");
  }
}

char pop_first(string & dna, string & rescue) {
  if (dna.length() < 1) {
    throw finish_exception("Not enough dna left to do anything");
  }
  char to_return = dna[0];
  shift_dna_left(dna, 1, rescue);
  return to_return;
}

unsigned int inty_nat(string & dna, string & rescue)
{
  if (dna.length() < 1) {
    throw finish_exception("dna not long enough to nat");
  }
  char first = pop_first(dna, rescue);
  switch (first) {
  case 'P':
    return 0;
    break;
  case 'C':
    return (2 * inty_nat(dna, rescue)) + 1;
    break;
  default: // I and F have the same result
    return (2 * inty_nat(dna, rescue));
    break;
  }
}

char nat(string & dna, string & rescue) //wraps real inty nat function
{
  const char zero('0');
  return inty_nat(dna, rescue) + zero;
}

string consts(string & dna, string & rescuable)
{
  char first = pop_first(dna, rescuable);
  switch (first) {
  case 'C':
    return "I" + consts(dna, rescuable);
    break;
  case 'F':
    return "C" + consts(dna, rescuable);
    break;
  case 'P':
    return "F" + consts(dna, rescuable);
    break;
  case 'I':
    char second = pop_first(dna, rescuable);
    if (second=='C')
      return "P" + consts(dna, rescuable);
    else 
      throw finish_exception("not enough dna to complete consts");
    break;
  }
  throw "compiler being silly";
}

void single_base_match(char to_match, string & dna, string & rna, string & rescuable, string & result_pattern, void (* default_function)(char, string &, string &, string &, string &))
{
  switch (to_match) {
  case 'C':
    result_pattern.push_back('I');
    break;
  case 'F':
    result_pattern.push_back('C');
    break;
  case 'P':
    result_pattern.push_back('F');
    break;
  case 'I':
    char second = pop_first(dna, rescuable);
    switch (second) {
    case 'C':
      result_pattern.push_back('P');
      break;
    default:
      default_function(second, dna, rna, rescuable, result_pattern);
      break;
    }
  }
}

string pattern(string & dna, string & rna) //evil, but I'm lazy...
{
  string result_pattern(""), rescuable("");
  bool finished(false);
  unsigned int level(0);

  while (!finished) {
    rescuable="";
    char first = pop_first(dna, rescuable);
    switch (first) {
    case 'C':
      result_pattern.push_back('I');
      break;
    case 'F':
      result_pattern.push_back('C');
      break;
    case 'P':
      result_pattern.push_back('F');
      break;
    case 'I':
      char second = pop_first(dna, rescuable);
      switch (second) {
      case 'C':
	result_pattern.push_back('P');
	break;
      case 'F':
	{
	  pop_first(dna, rescuable);
	  string s = consts(dna, rescuable);
	  result_pattern.push_back('?');
	  result_pattern.append(s);
	}
	  break;
      case 'P':
	{
	  char n = nat(dna, rescuable);
	  result_pattern.push_back('!');
	  result_pattern.push_back(n); //we don't want to add ! if finish exception is thrown.
	  break;
	}
      case 'I':
	char third = pop_first(dna, rescuable);
	switch (third) {
	case 'P':
	  ++level;
	  result_pattern.push_back('(');
	  break;
	case 'I':
	  push_dna_to_rna(dna, rna);
	  break;
	default: //C or F yield the same
	  if (level > 0) {
	    --level;
	    result_pattern.push_back(')');
	  }
	  else {
	    return result_pattern;
	  }
	  break;
	}
      }
      break;
    }
  }
  throw "compiler is silly";
}

/*

   function template () : Template =
     let t : Template ← ε;
     repeat
       case dna starts with
                          ⇒ dna ← dna[ 1 . . ]; t ← t I
           ‘C’
                          ⇒ dna ← dna[ 1 . . ]; t ← t C
           ‘F’
                          ⇒ dna ← dna[ 1 . . ]; t ← t F
           ‘P’
                          ⇒ dna ← dna[ 2 . . ]; t ← t P
           ‘IC’
           ‘IF’ or ‘IP’ ⇒ dna ← dna[ 2 . . ]; let l ← nat (); let n ← nat (); t ← t nl
           ‘IIC’ or ‘IIF’ ⇒ dna ← dna[ 3 . . ]; return t
                          ⇒ dna ← dna[ 3 . . ]; let n ← nat (); t ← t |n|
           ‘IIP’
                ⇒ rna ← rna dna[ 3 . . 10 ]; dna ← dna[ 10 . . ]
           ‘III’
           anything else ⇒ finish ()
       end case
     end repeat

 */

void template_default_function(char to_match, string & dna, string & rna, string & rescuable, string & result_pattern)
{
  if (to_match=='F' || to_match=='P') {
    char l = nat(dna, rescuable);
    char n = nat(dna, rescuable);
    result_pattern.push_back('[');
    result_pattern.push_back(n);
    result_pattern.push_back('_');
    result_pattern.push_back(l);
    result_pattern.push_back(']');
  }
  else { //char=='I'
    char third = pop_first(dna, rescuable);
    switch (third) {
    case 'I':
      {
	push_dna_to_rna(dna, rna);
      }
      break;
    case 'P':
      {
	char n = nat(dna, rescuable);
	result_pattern.push_back('|');
	result_pattern.push_back(n);
	result_pattern.push_back('|');
      }
      break;
    default: //C or F
      rescuable = ""; //stop last three eaten characters being readded by the catcher
      throw finish_exception("Cheating exception to stop us having to pass finished around");
      break;
    }
  }
}


string make_template(string & dna, string & rna)
{
  string result_pattern(""),rescuable("");
  bool finished(false);
  while (!finished) {
    try {
      rescuable = "";
      char first = pop_first(dna, rescuable);
      single_base_match(first, dna, rna, rescuable, result_pattern, &template_default_function);
    }
    catch (finish_exception &) {
      finished = true;
      dna.append(rescuable);
    }
  }
  return "moooo";
}


int main(int argc, char* argv[])
{
  string moo("moo"), whatever("");
  shift_dna_left(moo,1,whatever);
  assert(moo == "oo");
  shift_dna_left(moo,2,whatever);
  assert(moo == "");
  assert(whatever == "moo");

  string pomegranate("pomegranate");
  string not_really_rna("");

  push_dna_to_rna(pomegranate, not_really_rna);
  assert(pomegranate=="nate");
  assert(not_really_rna=="pomegra");

  string dna("CIIC");
  string rna("");
  
  assert("I"==pattern(dna, rna));
  assert("" == rna);
  assert("" == dna);

  dna = "IIPIPICPIICICIIF";
  rna = "";
  string our_pattern = pattern(dna,rna);
  assert ("(!2)P"==our_pattern);

}