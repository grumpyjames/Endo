#include <string>
#include <cstring>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <deque>
#include <list>
#include <utility>
#include "time.h"
#include "dna_string.h"
#include <math.h>
using std::string;
using std::ifstream;

namespace {
  double one_over_ln_2(1/(log(2.0f)));
  unsigned int const log2_ceil(unsigned int const k)
  {
    return ceil(static_cast<double>(k) * one_over_ln_2);
  }
}

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
      return to_return;
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

unsigned int nrcinty_nat(dna_string & dna) {
  char const  * start = dna.get_char_ptr();
  dna.skip_to_first('P');
  ++dna; //want the char after P
  location to_return_to;
  dna.save_position(to_return_to);
  --dna; //back at the P again
  unsigned int to_return(0);
  while(start != dna.get_char_ptr()) {
    --dna;
    if (dna.get()!='C')
      to_return = (2 * to_return);
    else
      to_return = (2 * to_return) + 1;
  }
  dna.load_position(to_return_to);
  return to_return;
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

void nrcconsts(dna_string & dna, string & to_write_to) {
  bool done(false);
  char last_char('X');
  while (!done) {
    char const first = dna.get();
    ++dna;
    if (last_char=='I') {
      if (first!='C') {
	dna-=2; //back out the last two chars
	return;
      }
      else {
	to_write_to.push_back('P');
      }
    }
    else {
      if (first=='C')
	to_write_to.push_back('I');
      else if (first=='F')
	to_write_to.push_back('C');
      else if (first=='P')
	to_write_to.push_back('F');
    }
    last_char = first;
  }
}

void nrcconsts(char *& dna, string & rescuable, string & to_write_to)
{
  bool done(false);
  char last_char('X');
  while (!done) {
    char first = dna[0];
    ++dna;
    if (last_char=='I') {
      if (first!='C') {
	dna-=2; //back out the last two chars
	return;
      }
      else
	to_write_to.push_back('P');
    }
    else {
      if (first=='C')
	to_write_to.push_back('I');
      else if (first=='F')
	to_write_to.push_back('C');
      else if (first=='P')
	to_write_to.push_back('F');
    }
    last_char = first;
  }
}

void make_template(dna_string & dna, dna_string & rna, dna_template & result_template)
{
  //FIXME suspect calls to strlen will hurt a LOT.
  while (dna.has_next()) {
    char const first = dna.get();
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
      char const second = dna.get();
      ++dna;
      switch (second) {
      case 'C':
	result_template.push_back(template_piece('P'));
	break;
      default:
	{
	  if (second=='F' || second=='P') {
	    unsigned int l = nrcinty_nat(dna);
	    unsigned int n = nrcinty_nat(dna);
	    result_template.push_back(template_piece(n,l));
	  }
	  else { //char=='I'
	    char const third = dna.get();
	    ++dna;
	    switch (third) {
	    case 'I':
	      {
		dna.push_to(rna, 8);
	      }
	      break;
	    case 'P':
	      {
		result_template.push_back(template_piece(nrcinty_nat(dna)));
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

void pattern(dna_string & dna, dna_string & rna, dna_pattern & result_pattern)
{
  string rescuable("");
  unsigned int level(0); 
  while (dna.has_next()) {
    rescuable="";
    char const first = dna.get();
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
      char const second = dna.get();
      ++dna;
      switch (second) {
      case 'C':
	result_pattern.push_back(pattern_piece('P'));
	break;
      case 'F':
	{
	  ++dna;
	  string consts("");
	  nrcconsts(dna, consts);
	  result_pattern.push_back(pattern_piece(consts));
	}
	  break;
      case 'P':
	{
	  result_pattern.push_back(pattern_piece(nrcinty_nat(dna)));
    	}
	break;
      case 'I':
	char const third = dna.get();
	++dna;
	switch (third) {
	case 'P':
	  ++level;
	  result_pattern.push_back(pattern_piece('('));
	  break;
	case 'I':
	  dna.push_to(rna, 7);
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
	  string consts("");
	  nrcconsts(dna, rescuable, consts);
	  result_pattern.push_back(pattern_piece(consts));
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
    to_write_to.push_back('C');
    asnat(floor(k/2), to_write_to);
  }
  else {
    to_write_to.push_back('I');
    asnat(floor(k/2), to_write_to);
  }
}

//my char * must have as least ceil(log_2(k)) chars left.
void asnat(unsigned int const k, char * to_write_to)
{
   if (k==0) {
    *to_write_to = 'P';
    return;
  }
  else if (k & 1) {
    *to_write_to = 'C';
    asnat(floor(k/2), ++to_write_to);
  }
  else {
    *to_write_to = 'I';
    asnat(floor(k/2), ++to_write_to);
  }
} 

void protect(unsigned int level, string & bits, string & to_write_to)
{
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

char * replace(char * & dna, dna_template to_replace, std::deque<string> env)
{
  string r("");
  for (unsigned int i=0; i<to_replace.size(); ++i) {
    switch (to_replace[i].type()) {
    case template_piece::N_L :
      {
	string to_append("");
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
  //FIXME pass in a string for new dna to be written to to avoid heap usage
  char * new_dna = new char[strlen(dna) + r.length()];
  memcpy(new_dna, r.c_str(), r.length());
  memcpy(new_dna + r.length(), dna, strlen(dna));
  //new_dna->reserve(strlen(dna) + r.length());
  //new_dna->append(r);
  //new_dna->append(dna);
  return new_dna;
}

void protect(unsigned int const level, dna_string & to_protect)
{
  if (level==0)
    return; //cheeky early return for the common case.
  //otherwise, fail - we'll have to allocate, and do some work
  //equally, I want to see if this ever actually gets called
  std::cerr << "Not implemented yet!" << '\n';
  throw "called protect with non zero l - not implemented yet!";
#if 0
  char * to_write_to = new char[to_protect.length() * 2]; //FIXME Collect anything we don't use.
  
  

  for (unsigned int i=0; i<level; ++i) {
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
#endif
}

void replace(dna_string & dna, std::deque<dna_string> & env, dna_template & to_replace)
{
  dna_string r;
  for (unsigned int i=0; i<to_replace.size(); ++i) {
    switch (to_replace[i].type()) {
    case template_piece::N_L :
      {
	dna_string to_protect = env[to_replace[i].n()];
	protect(to_replace[i].l(), to_protect);
	r.append(to_protect);
      }
      break;
    case template_piece::N :
      {
	unsigned int n = to_replace[i].n();
	unsigned int max_length(log2_ceil(n) + 1);
	char * to_append = new char[max_length];
	char * beginning = to_append;
	asnat(env[n].remaining_length(), to_append);
	ends some_ends(beginning, to_append);
      	r.push_back(some_ends);
      }
      break;
    case template_piece::SINGLE_BASE: //lone base
      r.push_back(to_replace[i].base()); //FIXME UGH! Do something about this.
      break;
    }
  }
  dna.prepend(r);
} 

void match_replace(dna_string & dna, dna_pattern & to_match, dna_template & to_replace)
{
  std::list<location> c;
  std::deque<dna_string> env;
  location start_location;
  dna.save_position(start_location);
  for (size_t s=0; s<to_match.size(); ++s) {
    switch (to_match[s].type()) {
    case pattern_piece::SKIP:
      {
	dna += (to_match[s].skip_length());
      }
      break;
    case pattern_piece::FIND:
      {
	dna.skip_to_first(to_match[s].search_term().c_str(), to_match[s].search_term().length());
	if (!dna.has_next()) {
	  dna.load_position(start_location);
	  return;
	}
      }
    case pattern_piece::BRACE_OPEN:
      c.push_front(dna.current_location());
      break;
    case pattern_piece::BRACE_CLOSE:
      {
	location moo = c.front();
	dna_string env_piece;
	dna.substr_from(moo, env_piece);
	env.push_back(env_piece);
	c.pop_front();
	break;
      }
    case pattern_piece::SINGLE_BASE: //I C F or P
      if (to_match[s].base()==dna.get())
	++dna;
      else
	throw "No Match";
      break;
    }
  }
  replace(dna, env, to_replace);
}

//FIXME does this need to be char * & ?
char * match_replace(char * & dna, dna_pattern & to_match, dna_template & to_replace)
{
  unsigned int i(0);
  std::list<unsigned int> c;
  std::deque<string> env;
  for (size_t s=0; s<to_match.size(); ++s) {
    switch (to_match[s].type()) {
    case pattern_piece::SKIP:
      {
	i += to_match[s].skip_length();
      }
      break;
    case pattern_piece::FIND:
      {
	char * haystack = dna + i;
	char const * found = strstr(haystack, to_match[s].search_term().c_str());
	if (found == NULL)
	  throw "No Match";
	else
	  i = strlen(dna)-strlen(found)+to_match[s].search_term().length();
	break;
      }
    case pattern_piece::BRACE_OPEN:
      c.push_front(i);
      break;
    case pattern_piece::BRACE_CLOSE:
      {
	string building("");
	unsigned int moo = c.front();
	building.reserve(i-moo);
	building.append(dna, moo, i-moo);
	env.push_back(building);
	c.pop_front();
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
  //std::cout << "successful match: i= " << i << '\n'; 
  dna+=i;
  // std::cout << "env[0] length is " << env[0].length() << " and starts with " << env[0].substr(0,10) << '\n';
  //std::cout << "env[1] length is " << env[1].length() << " and starts with " << env[1].substr(0,10) << '\n';
  return replace(dna, to_replace, env);
}

void alt_main(dna_string dna)
{
  std::cout << "Made it to alt main: dna string is this long: " << dna.remaining_length() << '\n';
  dna_string rna;
  dna_pattern our_pattern;
  pattern(dna, rna, our_pattern);
  display(our_pattern);
  dna_template our_template;
  make_template(dna, rna, our_template);
  display(our_template);
  match_replace(dna, our_pattern, our_template);
  std::cout << "Remaining dna length is " << dna.remaining_length() << '\n';
  std::cout << "RNA length is " << rna.remaining_length() << '\n';
  std::cout << "Leaving alt main" << '\n';
}

int main(int argc, char* argv[])
{
  //now do the real thing:
  ifstream ifs( "endo.dna" );
  string actual_dna;
  string actual_rna("");
  char * primitive_dna = new char[7523060];
  ifs.read(primitive_dna, 7523060);
  string pattern_holder(""),template_holder(""); 
  char * start_of_dna = primitive_dna;
  char * end_of_dna = primitive_dna + 7523059;
  ends initial(start_of_dna, end_of_dna);
  dna_string whole_dna;
  whole_dna.push_back(initial);
  alt_main(whole_dna);
  std::cout << strlen(primitive_dna) << '\n';
  for (unsigned int i(0); i< 200; ++i) {
    std::cout << '\n'<< "Iteration " << i << '\n'; 
    try {
      time_t begin,end;
      begin = time(NULL);
      dna_pattern a_pattern;
      pattern(primitive_dna, actual_rna, a_pattern);
      end = time(NULL);
      display(a_pattern);
      begin = end;
      dna_template a_template;
      cmake_template(primitive_dna, actual_rna, a_template);
      display(a_template);
      std::cout << "rna length: " << actual_rna.length() << '\n';
      end = time(NULL);
      try {
	char * new_dna;
	new_dna = match_replace(primitive_dna, a_pattern, a_template);
	primitive_dna = new_dna;
	delete[] start_of_dna;
	start_of_dna = primitive_dna;
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
  return 0;
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
