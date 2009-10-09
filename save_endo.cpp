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
#include <math.h>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>

#include "dna_string.h"

using std::string;
using std::ifstream;

char const I('I');
char const C('C');
char const F('F');
char const P('P');
char const OPEN('(');
char const CLOSE(')');

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
  pattern_piece(char const & base) { 
    if (base != OPEN && base != CLOSE) {
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
  char const & base() { return *base_; }
  string const & search_term() { return search_term_; }
  string const to_s();
private:
  boost::optional<char> base_;
  //FIXME make these optional?
  size_t type_;
  unsigned int skip_length_;
  string search_term_;
};

string const pattern_piece::to_s() {
  switch (this->type_) {
  case SKIP:
    {
      std::stringstream out;
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
      to_return.push_back(*base_);
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

//FIXME does this really need to be a deque?
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
  enum template_piece_type { N, N_L, DNA_STRING };
  template_piece(unsigned int n) : type_(N), n_(n) {}
  template_piece(unsigned int n, unsigned int l) : type_(N_L), n_(n), l_(l) {}
  template_piece(ends const & e) : type_(DNA_STRING), ends_(e) {}
  size_t const & type() { return type_;}
  unsigned int & n() { return n_; }
  unsigned int & l() { return l_; }
  ends const & dna_s() { return *ends_; }
  string to_s();
private:
  size_t const type_;
  unsigned int n_, l_;
  boost::optional<ends> const ends_;
};

string template_piece::to_s() {
  switch (type()) {
  case N_L:
    {
      std::stringstream out;
      out << "[" << n_ << "," << l_ << "]";
      return out.str();
    }
  case N:
    {
      std::stringstream out;
      out << "|" << n_ << "|";
      return out.str();
    }
  case DNA_STRING:
    {
      return ends_->to_s();
    }
  }
  throw ("sigh");
}

//FIXME Again, should this be a deque?
typedef std::deque<template_piece> dna_template;

void display(dna_template & a_template)
{
  std::cout << "Template is ";
  for (unsigned int i(0); i<a_template.size(); ++i) {
    std::cout << a_template[i].to_s();
  }
  std::cout << '\n';
}

class template_builder {
public:
  template_builder() : flushable(false),under_construction_(new dna_template) {}
  void operator<<(char const single_base) {
    ss_ << single_base;
    flushable = true;
  }
  void operator<<(unsigned int n) {
    flush();
    under_construction_->push_back(template_piece(n));
  }
  void operator<<(std::pair<unsigned int, unsigned int> const n_l) {
    flush();
    under_construction_->push_back(template_piece(n_l.first, n_l.second));
  }
  boost::shared_ptr<dna_template> to_template() { flush(); return under_construction_; }
private:
  //call this whenever a string of single bases ends.
  bool flushable;
  void flush() {
    if (!flushable)
      return;
    //FIXME this isn't terrifically efficient.
    flushable = false;
    string to_char_shortly = ss_.str();
    size_t dna_string_length = to_char_shortly.length();
    char * almost_there = new char[dna_string_length];
    memcpy(almost_there, to_char_shortly.c_str(), dna_string_length);
    under_construction_->push_back(template_piece(ends(almost_there, almost_there + (dna_string_length - 1), 0)));
    ss_.str("");
  }
  std::stringstream ss_;
  boost::shared_ptr<dna_template> under_construction_;
};

/*

ICPIFICPP

I->C
C->F
F->P
P->IC

1: CFICCPCFICIC
2. FPCFFICFPCFCF

CFICC


 */

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

unsigned int nrcinty_nat(dna_string & dna) {
  //quick return if we're at a P
  if (dna.get()=='P') {
    ++dna;
    return 0;
  }
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
	to_write_to.push_back(P);
      }
    }
    else {
      if (first=='C')
	to_write_to.push_back(I);
      else if (first=='F')
	to_write_to.push_back(C);
      else if (first=='P')
	to_write_to.push_back(F);
    }
    last_char = first;
  }
}

boost::shared_ptr<dna_template> make_template(dna_string & dna, dna_string & rna)
{
  template_builder tb;
  while (dna.has_next()) {
    char const first = dna.get();
    ++dna;
    switch (first) {
    case 'C':
      tb << 'I';
      break;
    case 'F':
      tb << 'C';
      break;
    case 'P':
      tb << 'F';
      break;
    case 'I':
      char const second = dna.get();
      ++dna;
      switch (second) {
      case 'C':
	tb << 'P';
	break;
      default:
	{
	  if (second=='F' || second=='P') {
	    unsigned int l = nrcinty_nat(dna);
	    unsigned int n = nrcinty_nat(dna);
	    tb << std::pair<unsigned int, unsigned int>(n, l);
	  }
	  else { //char=='I'
	    char const third = dna.get();
	    ++dna;
	    switch (third) {
	    case 'I':
	      {
		dna.push_to(rna, 7); //push 7 characters and move to the 8th.
	      }
	      break;
	    case 'P':
	      {
		tb << nrcinty_nat(dna);
	      }
	      break;
	    default: //C or F
	      return tb.to_template();
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
      result_pattern.push_back(pattern_piece(I));
      break;
    case 'F':
      result_pattern.push_back(pattern_piece(C));
      break;
    case 'P':
      result_pattern.push_back(pattern_piece(F));
      break;
    case 'I':
      char const second = dna.get();
      ++dna;
      switch (second) {
      case 'C':
	result_pattern.push_back(pattern_piece(P));
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
	  result_pattern.push_back(pattern_piece(OPEN));
	  break;
	case 'I':
	  dna.push_to(rna, 7);
	  break;
	default: //C or F yield the same
	  if (level > 0) {
	    --level;
	    result_pattern.push_back(pattern_piece(CLOSE));
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

void protect(unsigned int const level, dna_string & to_protect)
{
  if (level==0)
    return; //cheeky early return for the common case.
  else
    to_protect.protect(level);
  return;
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
	unsigned int n = to_replace[i].n();
	if (n < env.size()) {
	  dna_string to_protect = env[to_replace[i].n()];
	  protect(to_replace[i].l(), to_protect);
	  r.append(to_protect);
	}
      }
      break;
    case template_piece::N :
      {
	unsigned int n = to_replace[i].n();
	unsigned int max_length(log2_ceil(n) + 1);
	char * to_append = new char[max_length];
	char * beginning = to_append;
	asnat(env[n].remaining_length(), to_append);
	ends some_ends(beginning, to_append, 0);
      	r.push_back(some_ends);
      }
      break;
    case template_piece::DNA_STRING:
      {
	r.push_back(to_replace[i].dna_s());
      }
      break;
    }
  }
  std::cout << '\n';
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
	unsigned int skip_length = to_match[s].skip_length();
	if (dna.remaining_length() >= skip_length) { //we're allowed to be at the end of dna with no more chars
	  dna += (to_match[s].skip_length());
	}
	else {
	  dna.load_position(start_location);
	  return;
	}
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
      break;
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
      if (to_match[s].base()==dna.get()) {
	++dna;
      } else {
	std::cout << "Match not found!" << '\n';
	dna.load_position(start_location);
	return;
      }
      break;
    }
  }
  replace(dna, env, to_replace);
}

void test_dna_string_remaining_length() {
  char const * ten = "0123456789";
  ends eight(ten, ten+7, 0); //this is EIGHT chars
  ends two(ten+8,ten+9, 0); //this is TWO chars
  dna_string to_test;
  to_test.push_back(eight);
  assert(to_test.remaining_length()==8);
  ++to_test;
  assert(to_test.remaining_length()==7);
  to_test+=4;
  assert(to_test.remaining_length()==3);
  to_test.push_back(two);
  assert(to_test.remaining_length()==5);
  std::cout << "dna string length works correctly" << '\n';
}

void test_dna_string_push_to_and_get() {
  char const * moo = "IFPIIPFIFPCIFPIPCCIFPICIPIFPIP";
  ends const allinone(moo, moo+29, 0);
  dna_string to_test,rna;
  to_test.push_back(allinone);
  to_test.push_to(rna, 8);
  assert(rna.remaining_length()==8);
  assert(to_test.remaining_length()==22);
  assert(rna.get()=='I');
  rna+=7;
  assert(rna.get()=='I');
  assert(to_test.get()=='F');
  to_test+=1;
  assert(to_test.get()=='P');
  to_test.push_to(rna, 8);
  assert(rna.remaining_length()==9);
  assert(to_test.remaining_length()==13);
  std::cout << "Push to and get seem to work correctly" << '\n';
}

void test_dna_substr_from() {
  char const * twelve = "012345678912";
  char const * one = "A";
  char const * twenty = "BCDEFGHIJKLMNOPQRSTU";
  ends uno(twelve, twelve+11, 0);
  ends dos(one, one, 0);
  ends treise(twenty, twenty+19, 0);
  dna_string dna;
  dna.push_back(uno);
  dna.push_back(dos);
  dna.push_back(treise);
  assert(dna.remaining_length()==33);
  dna_string to_substr_to;
  location from(0, twelve+5);
  dna+=20;
  assert(dna.remaining_length()==13);
  dna.substr_from(from, to_substr_to);
  assert(to_substr_to.remaining_length()==15);
  std::cout << "substr from appears to work" << '\n';
}

void test_dna_string_search() {
  dna_string dna;
  char const * moo = "01AC45RA89ACRANAC";
  ends const mookery(moo, moo+16, 0);
  dna.push_back(mookery);
  dna.skip_to_first("ACRA", 4);
  assert(dna.remaining_length()==3);
  char const * moo2 = "RA4726387ACRA122";
  ends const rookery(moo2, moo2+15, 0);
  dna.push_back(rookery);
  dna.skip_to_first("ACRA", 4);
  assert(dna.remaining_length()==14);
  dna.skip_to_first("ACRA", 4);
  assert(dna.remaining_length()==3);
  std::cout << "skip to first appears to work" << '\n';
}

void test_dna_string() {
  test_dna_string_remaining_length();
  test_dna_string_push_to_and_get();
  test_dna_substr_from();
  test_dna_string_search();
  std::cout << "Tests all passed" << '\n' << '\n';
}

void test_iteration() {
  /*
    150 ‘IIPIPICPIICICIIFICCIFPPIICCFPC’ turns into ‘PICFC’ 
    151 ‘IIPIPICPIICICIIFICCIFCCCPPIICCFPC’ turns into ‘PIICCFCFFPC’ 
    152 ‘IIPIPIICPIICIICCIICFCFC’ turns into ‘I’
   */
  char * one = "IIPIPICPIICICIIFICCIFPPIICCFPC";
  char * two = "IIPIPICPIICICIIFICCIFCCCPPIICCFPC";
  char * three = "IIPIPIICPIICIICCIICFCFC";
  dna_string ds_one;
  dna_string ds_two;
  dna_string ds_three;
  dna_string rna;
  ds_one.push_back(ends(one,one - 1 + strlen(one), 0));
  ds_two.push_back(ends(two,two - 1 + strlen(two), 0));
  ds_three.push_back(ends(three,three - 1 + strlen(three), 0));
  dna_pattern p,q,r;
  pattern(ds_one, rna, p);
  display(p);
  boost::shared_ptr<dna_template> t = make_template(ds_one, rna);
  display(*t);
  match_replace(ds_one, p, *t);
  ds_one.display(std::cerr);
  assert(ds_one.remaining_length() == 5);
  pattern(ds_two, rna, q);
  display(q);
  boost::shared_ptr<dna_template> u = make_template(ds_two, rna);
  display(*u);
  match_replace(ds_two, q, *u);
  ds_two.display(std::cerr);
  pattern(ds_three, rna, r);
  display(r);
  boost::shared_ptr<dna_template> v = make_template(ds_three, rna);
  display(*v);
  match_replace(ds_three, r, *v);
  ds_three.display(std::cerr);
}

void alt_main(dna_string dna)
{
  test_dna_string();
  test_iteration();
  dna_string rna;
  for (size_t i(0); i < 100; ++i) {
    std::cout << "Iteration " << i << " dna string is this long: " << dna.remaining_length() << '\n';
    dna_pattern our_pattern;
    pattern(dna, rna, our_pattern);
    display(our_pattern);
    boost::shared_ptr<dna_template> our_template = make_template(dna, rna);
    display(*our_template);
    match_replace(dna, our_pattern, *our_template);
    std::cout << "Remaining dna length is " << dna.remaining_length() << '\n';
    std::cout << "RNA length is " << rna.remaining_length() << '\n' << '\n' << '\n';
  }
  std::cout << "Leaving alt main" << '\n';
  assert(false);
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
  char * primitive_dna = const_cast<char *>(actual_dna.c_str()); //FIXME Just read into a char buffer, this is a hack
  string * start_of_dna = &actual_dna;

  for (unsigned int i(0); i< 1000; ++i) {
    std::cout << '\n'<< "Iteration " << i << '\n'; 
    try {
      // std::cout << "First few bases are ";
//       for (unsigned int j(0); j<10; ++j) {
// 	std::cout << primitive_dna[j];
//       }
      //std::cout << " and dna length is " << strlen(primitive_dna) <<  '\n';
      time_t begin,end;
      begin = time(NULL);
      dna_pattern a_pattern;
      pattern(primitive_dna, actual_rna, a_pattern);
      end = time(NULL);
      //display(a_pattern);
      //std::cout << "Pattern execution took " << end - begin << " seconds" << '\n';
      //std::cout << "rna length: " << actual_rna.length() << '\n';
      begin = end;
      dna_template a_template;
      cmake_template(primitive_dna, actual_rna, a_template);
      //display(a_template);
      //std::cout << "rna length: " << actual_rna.length() << '\n';
      end = time(NULL);
      //std::cout << "Template execution took " << end - begin << " seconds" << '\n';      
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
  return 0;
}


