#ifndef RESULT_HPP
#define RESULT_HPP

/// Either Success or Failure
enum class status_t { SUCCESS = 0, FAILURE = 1 };

/// Return FAILURE if given is not SUCCESS.
#define CHECK_SUCCESS(x) if ((x) != status_t::SUCCESS) { return status_t::FAILURE; }

/// Exit process if given is FAILURE.
#define EXIT_ON_FAILURE(x) if ((x) == status_t::FAILURE) { printf("check failure: file %s, line %d\n", __FILE__, __LINE__); exit(1); }

/// Return FAILURE if given is false or 0.
#define CHECK_TRUE(x) if (!(x)) { return status_t::FAILURE; }

/// Return FAILURE if given is NULL.
#define CHECK_NULL(x) if ((x) == NULL) { return status_t::FAILURE; }

#endif