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

class pattern_piece {
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
  string const to_s();
private:
  size_t type_;
  //FIXME make these optional?
  char base_;
  unsigned int skip_length_;
  string search_term_;
};

string const pattern_piece::to_s() {
  switch (this->type_) {
  case SKIP:
    {
      std::stringstream out; //FIXME build/include boost and use lexical_cast?
      out << "!";
      out << skip_length();
      return out.str();
    }
  case FIND:
    {
      string to_return("");
      to_return.push_back('?');
      to_return.append(search_term_);
      break;
    }
  case SINGLE_BASE:
    {
      string to_return("");
      to_return.push_back(base_);
      return to_return;
      break;
    }
  case BRACE_OPEN:
    {
      return "(";
      break;
    }
  case BRACE_CLOSE:
    {
      return ")";
      break;
    }
  }
  return "dgnlskrnr";
}



typedef std::deque<pattern_piece> dna_pattern;

void display(dna_pattern & a_pattern)
{
  std::cout << "Pattern is ";
  for (unsigned int i(0); i < a_pattern.size(); ++i) {
    std::cout << a_pattern[i].to_s();
  }
  std::cout << '\n';
}

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
  string to_s();
private:
  size_t const type_;
  unsigned int n_, l_;
  char base_;
};

string template_piece::to_s() {
  switch (type()) {
  case SINGLE_BASE:
    { 
      string to_return;
      to_return.push_back(base_);
      return to_return;
    }
  case N_L:
    {
      std::stringstream out; //FIXME build/include boost and use lexical_cast?
      out << "[" << n_ << "," << l_ << "]";
      return out.str();
    }
  case N:
    {
      std::stringstream out; //FIXME build/include boost and use lexical_cast?
      out << "|" << n_ << "|";
      return out.str();
    }
  }
  throw ("sigh");
}

typedef std::deque<template_piece> dna_template;

void display(dna_template & a_template)
{
  std::cout << "Template is ";
  for (unsigned int i(0); i<a_template.size(); ++i) {
    std::cout << a_template[i].to_s();
  }
  std::cout << '\n';
}

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

void cpush_dna_to_rna(char *& dna, string & rna)
{
  //FIXME - this may not have enough chars to execute!
  rna.append(dna, 7);
  dna+=7;
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
  for (unsigned int i=0; i<level; ++i) {
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

string * replace(char * & dna, dna_template to_replace, std::deque<string> env)
{
  string r("");
  std::cout << "Replacing template has length " <<  to_replace.size() << '\n';
  for (unsigned int i=0; i<to_replace.size(); ++i) {
    switch (to_replace[i].type()) {
    case template_piece::N_L :
      {
	string to_append("");
	std::cout << "about to do shit with protect: l = " << to_replace[i].l() << " n is " << to_replace[i].n() << " length of string being acted on is " << env[to_replace[i].n()].length() <<'\n';
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
  //FIXME Getting close to UB here.
  //FIXME pass in a string for new dna to be written to to avoid heap usage?
  string * new_dna = new string(""); //we're going to return a pointer to the c_str
  unsigned int new_dna_length = strlen(dna) + r.length();
  std::cout << new_dna_length << " is the new DNA length" << '\n';
  new_dna->reserve(strlen(dna) + r.length());
  new_dna->append(r);
  new_dna->append(dna);
  return new_dna;
}

//FIXME does this need to be char * & ?
string * match_replace(char * & dna, dna_pattern to_match, dna_template to_replace)
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
	  throw "No Match";
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
	throw "No Match";
      break;
    }
  }
  std::cout << "successful match: i= " << i << '\n'; 
  dna+=i;
  std::cout << "env[0] length is " << env[0].length() << " and starts with " << env[0].substr(0,10) << '\n';
  std::cout << "env[1] length is " << env[1].length() << " and starts with " << env[1].substr(0,10) << '\n';
  return replace(dna, to_replace, env);
}

int main(int argc, char* argv[])
{
  //now do the real thing:
  ifstream ifs( "endo.dna" );
  string actual_dna;
  string actual_rna("");

  std::cout << "max string size is " << actual_rna.max_size() << '\n';

  getline( ifs, actual_dna );
  string pattern_holder(""),template_holder(""); 
  char * primitive_dna = const_cast<char *>(actual_dna.c_str()); //FIXME Just read into a char buffer, this is a hack
  string * start_of_dna = &actual_dna;

  for (unsigned int i(0); i< 10; ++i) {
    try {
      std::cout << "First few bases are ";
      for (unsigned int j(0); j<10; ++j) {
	std::cout << primitive_dna[j];
      }
      std::cout << '\n';
      time_t begin,end;
      begin = time(NULL);
      dna_pattern a_pattern;
      pattern(primitive_dna, actual_rna, a_pattern);
      end = time(NULL);
      display(a_pattern);
      std::cout << "Pattern execution took " << end - begin << " seconds" << '\n';
      std::cout << "rna length: " << actual_rna.length() << '\n';
      begin = end;
      dna_template a_template;
      cmake_template(primitive_dna, actual_rna, a_template);
      display(a_template);
      std::cout << "rna length: " << actual_rna.length() << '\n';
      end = time(NULL);
      std::cout << "Template execution took " << end - begin << " seconds" << '\n';
      
      try {
	string * new_dna;
	new_dna = match_replace(primitive_dna, a_pattern, a_template);
	//delete start_of_dna; //FIXME this is getting seriously leaky!
	//start_of_dna = new_dna;
	primitive_dna = const_cast<char *>(new_dna->c_str());
      }
      catch (...) {
	std::cout << "No match found" << '\n';
      }
    }
    catch (finish_exception & fe)
      {
	std::cout << "caught finished exc "<< fe.why() << '\n';
      }
  }
  exit(0);
}

void test_pattern(char * primitive_dna) {
  time_t start, fin;
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
