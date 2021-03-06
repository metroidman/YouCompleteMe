// Copyright (C) 2011, 2012  Strahinja Val Markovic  <val@markovic.io>
//
// This file is part of YouCompleteMe.
//
// YouCompleteMe is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// YouCompleteMe is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with YouCompleteMe.  If not, see <http://www.gnu.org/licenses/>.

#include "CandidateRepository.h"
#include "Candidate.h"
#include "standard.h"
#include "Utils.h"

#include <boost/thread/locks.hpp>

#ifdef USE_CLANG_COMPLETER
#  include "CompletionData.h"
#endif // USE_CLANG_COMPLETER

namespace YouCompleteMe {

boost::mutex CandidateRepository::singleton_mutex_;
CandidateRepository *CandidateRepository::instance_ = NULL;

CandidateRepository &CandidateRepository::Instance() {
  boost::lock_guard< boost::mutex > locker( singleton_mutex_ );

  if ( !instance_ ) {
    static CandidateRepository repo;
    instance_ = &repo;
  }

  return *instance_;
}


int CandidateRepository::NumStoredCandidates() {
  boost::lock_guard< boost::mutex > locker( holder_mutex_ );
  return candidate_holder_.size();
}


std::vector< const Candidate * > CandidateRepository::GetCandidatesForStrings(
  const std::vector< std::string > &strings ) {
  std::vector< const Candidate * > candidates;
  candidates.reserve( strings.size() );

  {
    boost::lock_guard< boost::mutex > locker( holder_mutex_ );

    foreach ( const std::string & candidate_text, strings ) {
      const Candidate *&candidate = GetValueElseInsert( candidate_holder_,
                                                        candidate_text,
                                                        NULL );

      if ( !candidate )
        candidate = new Candidate( candidate_text );

      candidates.push_back( candidate );
    }
  }

  return candidates;
}

#ifdef USE_CLANG_COMPLETER

std::vector< const Candidate * > CandidateRepository::GetCandidatesForStrings(
  const std::vector< CompletionData > &datas ) {
  std::vector< const Candidate * > candidates;
  candidates.reserve( datas.size() );

  {
    boost::lock_guard< boost::mutex > locker( holder_mutex_ );

    foreach ( const CompletionData & data, datas ) {
      const Candidate *&candidate = GetValueElseInsert( candidate_holder_,
                                                        data.original_string_,
                                                        NULL );

      if ( !candidate )
        candidate = new Candidate( data.original_string_ );

      candidates.push_back( candidate );
    }
  }

  return candidates;
}

#endif // USE_CLANG_COMPLETER

CandidateRepository::~CandidateRepository() {
  foreach ( const CandidateHolder::value_type & pair,
            candidate_holder_ ) {
    delete pair.second;
  }
}

} // namespace YouCompleteMe
