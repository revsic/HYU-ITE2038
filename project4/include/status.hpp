#ifndef RESULT_HPP
#define RESULT_HPP

/// Either Success or Failure
enum class Status { SUCCESS = 0, FAILURE = 1 };

/// Return FAILURE if given is not SUCCESS.
#define CHECK_SUCCESS(x) if ((x) != Status::SUCCESS) { return Status::FAILURE; }

/// Exit process if given is FAILURE.
#define EXIT_ON_FAILURE(x) if ((x) == Status::FAILURE) { printf("check failure: file %s, line %d\n", __FILE__, __LINE__); exit(1); }

/// Return FAILURE if given is false or 0.
#define CHECK_TRUE(x) if (!(x)) { return Status::FAILURE; }

/// Exit process if given is FALSE.
#define EXIT_ON_FALSE(x) if (!(x)) { printf("check failure: file %s, line %d\n", __FILE__, __LINE__); exit(1); }

/// Return FAILURE if given is NULL.
#define CHECK_NULL(x) if ((x) == nullptr) { return Status::FAILURE; }

/// Exit process if given is NULL.
#define EXIT_ON_NULL(x) if ((x) == nullptr) { printf("check failure: file %s, line %d\n", __FILE__, __LINE__); exit(1); }

#endif