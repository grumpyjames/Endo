#include <string>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <deque>
#include <list>
#include "time.h"

using std::string;
using std::ifstream;

class  pattern_piece {
public:
  enum pattern_piece_type {
    SINGLE_BASE,
    SKIP,
    FIND,
    BRACE_OPEN,
    BRACE_CLOSE
  };
  pattern_piece(unsigned int const skip_length) : type_(SKIP), skip_length_(skip_length){}
  pattern_piece(char const base) { 
    if (base != '(' && base != ')') {
      type_ = SINGLE_BASE;
      base_ = base;
    }
    else if (base != ')') {
      type_ = BRACE_OPEN;
    }
    else {
      type_ = BRACE_CLOSE;
    }
  }
  pattern_piece(string const & search_term) : type_(FIND), search_term_(search_term){} 
  size_t const & type() { return type_; }
  unsigned int const & skip_length() { return skip_length_;}
  char const & base() { return base_; }
  string const & search_term() { return search_term_; }
  string const to_string();
private:
  size_t type_;
  //FIXME make these optional?
  char base_;
  unsigned int skip_length_;
  string search_term_;
};

string const pattern_piece::to_string() {
  switch (this->type_) {
  case SKIP:
    break;
  case FIND:
    break;
  case SINGLE_BASE:
    break;
  case BRACE_OPEN:
    break;
  case BRACE_CLOSE:
    break;
  }
  return "dgnlskrnr";
}



typedef std::deque<pattern_piece> dna_pattern;

class template_piece {
public:
  enum template_piece_type { SINGLE_BASE, N, N_L };
  template_piece(unsigned int n) : type_(N), n_(n) {}
  template_piece(unsigned int n, unsigned int l) : type_(N_L), n_(n), l_(l) {}
  template_piece(char base) : type_(SINGLE_BASE), base_(base) {}
  size_t const & type() { return type_;}
  unsigned int & n() { return n_; }
  unsigned int & l() { return l_; }
  char & base() { return base_; }
private:
  size_t const type_;
  unsigned int n_, l_;
  char base_;
};

typedef std::deque<template_piece> dna_template;

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

char * buffer = new char[7];

void cpush_dna_to_rna(char *& dna, string & rna)
{
  //FIXME - this may not have enough chars to execute!
  rna.append(dna, 7);
  dna+=7;
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

//non recursive
unsigned int nrcinty_nat(char *& dna, string & rescue)
{
  char * p_location = strchr(dna, 'P');
  char * point_dna_to_me_at_the_end = (p_location + 1);
  if (p_location==NULL) {
    throw finish_exception("could not find nat terminator");
  }
  //now we have a safe character pointer, and we should be allowed to decrement it back to dna
  unsigned int to_return(0);
  while(p_location != dna) {
    --p_location;
    if (p_location[0]=='C')
      to_return = (2 * to_return) + 1;
    else
      to_return = (2 * to_return);
  }
  dna = point_dna_to_me_at_the_end;
  return to_return;
}


string nat(string & dna, string & rescue) //wraps real inty nat function
{
  std::stringstream out; //FIXME build/include boost and use lexical_cast?
  out << inty_nat(dna, rescue);
  return out.str();
}

string cnat(char * & dna, string & rescue)
{
  std::stringstream out; //FIXME build/include boost and use lexical_cast?
  out << nrcinty_nat(dna, rescue);
  return out.str();
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

string cconsts(char *& dna, string & rescuable)
{
  char first = dna[0];
  ++dna;
  switch (first) {
  case 'C':
    return "I" + cconsts(dna, rescuable);
    break;
  case 'F':
    return "C" + cconsts(dna, rescuable);
    break;
  case 'P':
    return "F" + cconsts(dna, rescuable);
    break;
  case 'I':
    char second = dna[0];
    ++dna;
    if (second=='C')
      return "P" + cconsts(dna, rescuable);
    else 
      return "";//throw finish_exception("not enough dna to complete consts");
    break;
  }
  throw "compiler being silly";

}

string make_template(string & dna, string & rna)
{
  string result_pattern;
  string rescuable;
  while (dna.length()>0) {
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
      default:
	{
	  if (second=='F' || second=='P') {
	    string l = nat(dna, rescuable);
	    string n = nat(dna, rescuable);
	    result_pattern.push_back('[');
	    result_pattern.append(n);
	    result_pattern.push_back('_');
	    result_pattern.append(l);
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
		string n = nat(dna, rescuable);
		result_pattern.push_back('|');
		result_pattern.append(n);
		result_pattern.push_back('|');
	      }
	      break;
	    default: //C or F
	      return result_pattern;
	    }
	  }
	  break;
	}
      }
    }
  }
  throw finish_exception("ran out of dna!");
}

void cmake_template(char *& dna, string & rna, dna_template & result_template)
{
  string rescuable;
  rna.reserve(strlen(dna));
  //FIXME suspect calls to strlen will hurt a LOT.
  while (true) { //strlen(dna)>0) { commented for memory
    char first = dna[0];
    ++dna;
    switch (first) {
    case 'C':
      result_template.push_back(template_piece('I'));
      break;
    case 'F':
      result_template.push_back(template_piece('C'));
      break;
    case 'P':
      result_template.push_back(template_piece('F'));
      break;
    case 'I':
      char second = dna[0];
      ++dna;
      switch (second) {
      case 'C':
	result_template.push_back(template_piece('P'));
	break;
      default:
	{
	  if (second=='F' || second=='P') {
	    unsigned int l = nrcinty_nat(dna, rescuable);
	    unsigned int n = nrcinty_nat(dna, rescuable);
	    result_template.push_back(template_piece(n,l));
	  }
	  else { //char=='I'
	    char third = dna[0];
	    ++dna;
	    switch (third) {
	    case 'I':
	      {
		cpush_dna_to_rna(dna, rna);
	      }
	      break;
	    case 'P':
	      {
		result_template.push_back(template_piece(nrcinty_nat(dna, rescuable)));
	      }
	      break;
	    default: //C or F
	      return;
	    }
	  }
	  break;
	}
      }
    }
  }
  throw finish_exception("ran out of dna!");
}

string cmake_template(char *& dna, string & rna)
{
  string result_pattern;
  string rescuable;
  rna.reserve(strlen(dna));
  //FIXME suspect calls to strlen will hurt a LOT.
  while (true) { //strlen(dna)>0) { commented for memory
    char first = dna[0];
    ++dna;
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
      char second = dna[0];
      ++dna;
      switch (second) {
      case 'C':
	result_pattern.push_back('P');
	break;
      default:
	{
	  if (second=='F' || second=='P') {
	    string l = cnat(dna, rescuable);
	    string n = cnat(dna, rescuable);
	    result_pattern.push_back('[');
	    result_pattern.append(n);
	    result_pattern.push_back('_');
	    result_pattern.append(l);
	    result_pattern.push_back(']');
	  }
	  else { //char=='I'
	    char third = dna[0];
	    ++dna;
	    switch (third) {
	    case 'I':
	      {
		cpush_dna_to_rna(dna, rna);
	      }
	      break;
	    case 'P':
	      {
		string n = cnat(dna, rescuable);
		result_pattern.push_back('|');
		result_pattern.append(n);
		result_pattern.push_back('|');
	      }
	      break;
	    default: //C or F
	      return result_pattern;
	    }
	  }
	  break;
	}
      }
    }
  }
  throw finish_exception("ran out of dna!");
}

void pattern(char *& dna, string & rna, dna_pattern & result_pattern)
{
  string rescuable("");
  bool finished(false);
  unsigned int level(0); 
  rna.reserve(strlen(dna));
  while (!finished) {
    rescuable="";
    char first = dna[0];
    ++dna;
    switch (first) {
    case 'C':
      result_pattern.push_back(pattern_piece('I'));
      break;
    case 'F':
      result_pattern.push_back(pattern_piece('C'));
      break;
    case 'P':
      result_pattern.push_back(pattern_piece('F'));
      break;
    case 'I':
      char second = dna[0];
      ++dna;
      switch (second) {
      case 'C':
	result_pattern.push_back(pattern_piece('P'));
	break;
      case 'F':
	{
	  ++dna;
	  result_pattern.push_back(pattern_piece(cconsts(dna, rescuable)));
	}
	  break;
      case 'P':
	{
	  result_pattern.push_back(pattern_piece(nrcinty_nat(dna, rescuable)));
    	}
	break;
      case 'I':
	char third = dna[0];
	++dna;
	switch (third) {
	case 'P':
	  ++level;
	  result_pattern.push_back(pattern_piece('('));
	  break;
	case 'I':
	  cpush_dna_to_rna(dna, rna);
	  break;
	default: //C or F yield the same
	  if (level > 0) {
	    --level;
	    result_pattern.push_back(pattern_piece(')'));
	  }
	  else {
	    return;
	  }
	  break;
	}
      }
      break;
    }
  }
  throw "compiler is silly";
}

string pattern(char *& dna, string & rna) //evil, but I'm lazy...
{
  string result_pattern(""), rescuable("");
  bool finished(false);
  unsigned int level(0); 
  rna.reserve(strlen(dna));
  while (!finished) {
    rescuable="";
    char first = dna[0];
    ++dna;
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
      char second = dna[0];
      ++dna;
      switch (second) {
      case 'C':
	result_pattern.push_back('P');
	break;
      case 'F':
	{
	  ++dna;
	  string s = cconsts(dna, rescuable);
	  result_pattern.push_back('?');
	  result_pattern.append(s);
	  result_pattern.push_back('/');
	}
	  break;
      case 'P':
	{
	  string n = cnat(dna, rescuable);
	  result_pattern.push_back('!');
	  result_pattern.append(n); //we don't want to add ! if finish exception is thrown.
	  result_pattern.push_back('/'); //easier to find a terminator
	}
	break;
      case 'I':
	char third = dna[0];
	++dna;
	switch (third) {
	case 'P':
	  ++level;
	  result_pattern.push_back('(');
	  break;
	case 'I':
	  cpush_dna_to_rna(dna, rna);
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

//recursive call is ok - we're not allocating.
void asnat(unsigned int k, std::string & to_write_to)
{
  if (k==0) {
    to_write_to.push_back('P');
    return;
  }
  else if (k & 1) {
    to_write_to.push_back('I');
    asnat(floor(k/2), to_write_to);
  }
  else {
    to_write_to.push_back('C');
    asnat(floor(k/2), to_write_to);
  }
}

void protect(unsigned int level, string & bits, string & to_write_to)
{
  //std::cout << "bits is " << bits <<'\n';
  for (unsigned int i=0; i<=level; ++i) {
    string temp("");
    temp.reserve(bits.length() * 2);
    for (unsigned int j=0; j<bits.length(); ++j){
      switch (bits[i]) {
      case 'I':
	temp.push_back('C');
	break;
      case 'C':
	temp.push_back('F');
      break;
      case 'F':
	temp.push_back('P');
	break;
      case 'P':
	temp.append("IC");
	break;
      default:
	break;
      }
    }
    bits = temp;
    temp = "";
  }
  to_write_to = bits;
}


void replace(char * dna, string & a_template, std::deque<string> & env)
{
  string r("");
  std::cout << "Replacing template has length " <<  a_template.length() << '\n';
  for (unsigned int i=0; i<a_template.length(); ++i) {
    switch (a_template[i]) {
    case '[':
      {
	++i; //we want to start the search at the next char
	size_t found = a_template.find('_', i);
	unsigned int n;
	std::cout << "finding template substring between chars " << i << " and " << (found) << '\n';
	std::istringstream a_stream(a_template.substr(i, found-i));
	a_stream >> n;
	std::cout << "found n was " <<  n << '\n';
	i = ++found;
	found = a_template.find(']', i);
	unsigned int l;
	std::cout << "finding template substring between chars " << i << " and " << (found) << '\n';
	std::istringstream b_stream(a_template.substr(i, found-i));
	b_stream >> l;
	std::cout << "found l was " <<  l << '\n';
	i = found;
	string to_append("");
	std::cout << "about to do shit with protect" <<'\n';
	protect(l, env[n], to_append);
	r.append(to_append);
      }
      break;
    case '|':
      {
	++i;
	size_t found = a_template.find('|', i) - 1;
	unsigned int n;
	std::istringstream a_stream(a_template.substr(i, found));
	a_stream >> n;
	i = ++found; //should move us to the final |
	string to_append;
	asnat(env[n].length(), to_append);
	r.append(to_append);
      }
      break;
    default: //lone base
      r.push_back(a_template[i]);
      break;
    }
  }
  std::cout << "r was " << r << '\n';
}

void replace(char * & dna, dna_template to_replace, std::deque<string> env)
{
  string r("");
  std::cout << "Replacing template has length " <<  to_replace.size() << '\n';
  for (unsigned int i=0; i<to_replace.size(); ++i) {
    switch (to_replace[i].type()) {
    case template_piece::N_L :
      {
	string to_append("");
	std::cout << "about to do shit with protect" <<'\n';
	protect(to_replace[i].l(), env[to_replace[i].n()], to_append);
	r.append(to_append);
      }
      break;
    case template_piece::N :
      {
	unsigned int n = to_replace[i].n();
	string to_append("");
	asnat(env[n].length(), to_append);
	r.append(to_append);
      }
      break;
    case template_piece::SINGLE_BASE: //lone base
      r.push_back(to_replace[i].base());
      break;
    }
  }
  std::cout << "r was " << r << '\n';
  string new_dna("");
  unsigned int new_dna_length = strlen(dna) + r.length();
  std::cout << new_dna_length << " is the new DNA length" << '\n';
  new_dna.reserve(strlen(dna) + r.length());
  new_dna.append(r);
  new_dna.append(dna);
  delete[] dna;
  dna = const_cast<char *>(new_dna.c_str());
}


void match_replace(char * & dna, dna_pattern to_match, dna_template to_replace)
{
  unsigned int i(0);
  std::list<unsigned int> c;
  std::deque<string> env;
  //std::cout << strlen(dna) << " - dna length" << '\n';
  for (size_t s=0; s<to_match.size(); ++s) {
    switch (to_match[s].type()) {
    case pattern_piece::SKIP:
      {
	i += to_match[s].skip_length();
      }
      break;
    case pattern_piece::FIND:
      {
	char * found = strstr(dna, to_match[s].search_term().c_str());
	if (found == NULL)
	  return;
	else
	  i = strlen(dna)-strlen(found);
	break;
      }
    case pattern_piece::BRACE_OPEN:
      //std::cout << i << " was pushed to the front of c" << '\n';
      c.push_front(i);
      break;
    case pattern_piece::BRACE_CLOSE:
      {
	//std::cout << "Closing a brace" << '\n';
	string building("");
	//std::cout << "Instantiating a string" << '\n';
	unsigned int moo = c.front();
	building.reserve(i-moo);
	//std::cout << "DNA length: " << strlen(dna) << ". Copying " << (i - moo) << " chars to building from position " << moo << '\n';
	building.append(dna, moo, i-moo);
	//std::cout << dna[moo] << " is the " << moo << "th dna character" << '\n';
	//std::cout << "done it, building is now " << building << '\n';
	env.push_back(building);
	//std::cout << "Adding the building to the env" << '\n';
	c.pop_front();
	//std::cout << c.front() << " is now the front of c" << '\n';
	//std::cout << "Removing the position from the list" << '\n';
	break;
      }
    case pattern_piece::SINGLE_BASE: //I C F or P
      if (to_match[s].base()==dna[i])
	++i;
      else
	return;
      break;
    }
  }
  std::cout << "successful match: i= " << i << '\n'; 
  dna+=i;
  std::cout << "env[0] length is " << env[0].length() << " and starts with " << env[0].substr(0,10) << '\n';
  std::cout << "env[1] length is " << env[1].length() << " and starts with " << env[1].substr(0,10) << '\n';
  replace(dna, to_replace, env);
}



void match_replace(char *& dna, string pattern, string a_template)
{
  unsigned int i(0);
  std::list<unsigned int> c;
  std::deque<string> env;
  //std::cout << strlen(dna) << " - dna length" << '\n';
  for (size_t s=0; s<pattern.length(); ++s) {
    switch (pattern[s]) {
    case '!':
      {
	++s;
	size_t end_of_nat = pattern.find('/', s) - 1;
	string n = pattern.substr(s,end_of_nat-s+1);
	//std::cout << "found nat: " << n << " end of nat " << end_of_nat << " s " << s << '\n';
	s = end_of_nat;
	i += atoi(n.c_str()); //evil, but probably quicker than streams: TEST!
	//std::cout << "i is now " << i << '\n';
      }
      break;
    case '?':
      {
	++s;
	size_t end_of_consts = pattern.find('/',s) - 1;
	string n = pattern.substr(s,end_of_consts-s+1);
	char * found = strstr(dna, n.c_str());
	if (found == NULL)
	  return;
	else
	  i = strlen(dna)-strlen(found);
	break;
      }
    case '(':
      //std::cout << i << " was pushed to the front of c" << '\n';
      c.push_front(i);
      break;
    case ')':
      {
	//std::cout << "Closing a brace" << '\n';
	string building("");
	//std::cout << "Instantiating a string" << '\n';
	unsigned int moo = c.front();
	building.reserve(i-moo);
	//std::cout << "DNA length: " << strlen(dna) << ". Copying " << (i - moo) << " chars to building from position " << moo << '\n';
	building.append(dna, moo, i-moo);
	//std::cout << dna[moo] << " is the " << moo << "th dna character" << '\n';
	//std::cout << "done it, building is now " << building << '\n';
	env.push_back(building);
	//std::cout << "Adding the building to the env" << '\n';
	c.pop_front();
	//std::cout << c.front() << " is now the front of c" << '\n';
	//std::cout << "Removing the position from the list" << '\n';
	break;
      }
    case '/':
      break;
    default: //I C F or P
      if (pattern[s]==dna[i])
	++i;
      else
	return;
      break;
    }
  }
  std::cout << "successful match: i= " << i << '\n'; 
  dna+=i;
  std::cout << "env[0] length is " << env[0].length() << " and starts with " << env[0].substr(0,10) << '\n';
  std::cout << "env[1] length is " << env[1].length() << " and starts with " << env[1].substr(0,10) << '\n';
  replace(dna, a_template, env);
}

void test()
{

  string pomegranate("pomegranate");
  string not_really_rna("");
  
  push_dna_to_rna(pomegranate, not_really_rna);
  assert(pomegranate=="nate");
  assert(not_really_rna=="pomegra");
  
  char * dna = "CIIC";
  string rna("");
  
  assert("I"==pattern(dna, rna));
  assert("" == rna);
  
  char * cdna = "IIPIPICPIICICIIF";
  
  rna = "";
  string our_pattern = pattern(cdna,rna);
  assert ("(!2/)P"==our_pattern);
}

int main(int argc, char* argv[])
{
  test();
  //now do the real thing:
  ifstream ifs( "endo.dna" );
  string actual_dna;
  string actual_rna("");

  std::cout << "max string size is " << actual_rna.max_size() << '\n';

  getline( ifs, actual_dna );
  std::cout << "Hey, I read in the dna, and the first few bases were: " << actual_dna.substr(4526645,800) << '\n';
  string pattern_holder(""),template_holder(""); 
  char * primitive_dna = const_cast<char *>(actual_dna.c_str());

  try {
    time_t begin,end;
    begin = time(NULL);
    pattern_holder = pattern(primitive_dna, actual_rna);
    std::cout << "pattern: " << pattern_holder << '\n';
    std::cout << "pattern length was " << pattern_holder.length() << '\n';
    end = time(NULL);
    std::cout << "Pattern execution took " << end - begin << " seconds" << '\n';
    std::cout << "rna length: " << actual_rna.length() << '\n';
    begin = end;
    template_holder = cmake_template(primitive_dna, actual_rna);
    std::cout << "template: " << template_holder << '\n';
    std::cout << "rna length: " << actual_rna.length() << '\n';
    end = time(NULL);
    std::cout << "Template execution took " << end - begin << " seconds" << '\n';
    match_replace(primitive_dna, pattern_holder, template_holder);
  }
  catch (finish_exception & fe)
    {
      std::cout << "caught finished exc "<< fe.why() << '\n';
    }
  exit(0);
}

void test_pattern(char * primitive_dna) {
  time_t start, fin;
  start = time(NULL);
  for (unsigned int i(0); i<1000; ++i){
    char * dna_copy = primitive_dna;
    string meh_rna, meh_pattern;
    meh_pattern = pattern(dna_copy, meh_rna);
  }
  fin = time(NULL);
  std::cout << "Crappy pattern took " << fin - start << " seconds" << '\n';
  start = time(NULL);
  for (unsigned int i(0); i<1000; ++i){
    char * dna_copy = primitive_dna;
    string meh_rna;
    dna_pattern result;
    pattern(dna_copy, meh_rna, result);
  }
  fin=time(NULL);
  std::cout << "Decent pattern took " << fin - start << " seconds" << '\n';
}
