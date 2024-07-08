// ------------------------------------------------------------------------
//
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2013 - 2024 by the deal.II authors
//
// This file is part of the deal.II library.
//
// Part of the source code is dual licensed under Apache-2.0 WITH
// LLVM-exception OR LGPL-2.1-or-later. Detailed license information
// governing the source code and code contributions can be found in
// LICENSE.md and CONTRIBUTING.md at the top level directory of deal.II.
//
// ------------------------------------------------------------------------


// same as gmres_reorthogonalize_01 but forces re-orthogonalization.

#include <deal.II/lac/full_matrix.h>
#include <deal.II/lac/precondition.h>
#include <deal.II/lac/solver_gmres.h>
#include <deal.II/lac/vector.h>

#include "../tests.h"



template <typename number>
void
test(unsigned int variant)
{
  const unsigned int n = 64;
  Vector<number>     rhs(n), sol(n);
  rhs = 1.;

  FullMatrix<number> matrix(n, n);
  for (unsigned int i = 0; i < n; ++i)
    for (unsigned int j = 0; j < n; ++j)
      matrix(i, j) = random_value<double>(-.1, .1);

  // put diagonal entries of different strengths. these are very challenging
  // for GMRES and will usually take a lot of iterations until the Krylov
  // subspace is complete enough
  if (variant == 0)
    for (unsigned int i = 0; i < n; ++i)
      matrix(i, i) = (i + 1);
  else if (variant == 1)
    for (unsigned int i = 0; i < n; ++i)
      matrix(i, i) = (i + 1) * (i + 1) * (i + 1) * (i + 1);
  else if (variant == 2)
    for (unsigned int i = 0; i < n; ++i)
      matrix(i, i) = 1e10 * (i + 1);
  else if (variant == 3)
    for (unsigned int i = 0; i < n; ++i)
      matrix(i, i) = 1e10 * (i + 1) * (i + 1) * (i + 1) * (i + 1);
  else if (variant == 4)
    for (unsigned int i = 0; i < n; ++i)
      matrix(i, i) = 1e30 * (i + 1);
  else if (variant == 5)
    for (unsigned int i = 0; i < n; ++i)
      matrix(i, i) = 1e30 * (i + 1) * (i + 1) * (i + 1) * (i + 1);
  else
    Assert(false, ExcMessage("Invalid variant"));
  if (std::is_same_v<number, float> == true)
    Assert(variant < 4, ExcMessage("Invalid_variant"));

  deallog.push(Utilities::int_to_string(variant, 1));

  SolverControl control(1000, 1e2 * std::numeric_limits<number>::epsilon());
  typename SolverGMRES<Vector<number>>::AdditionalData data;
  data.max_basis_size             = 80;
  data.force_re_orthogonalization = true;
  data.orthogonalization_strategy =
    LinearAlgebra::OrthogonalizationStrategy::modified_gram_schmidt;

  SolverGMRES<Vector<number>> solver(control, data);
  auto print_re_orthogonalization = [](int accumulated_iterations) {
    deallog.get_file_stream() << "Re-orthogonalization enabled at step "
                              << accumulated_iterations << std::endl;
  };
  solver.connect_re_orthogonalization_slot(print_re_orthogonalization);
  solver.solve(matrix, sol, rhs, PreconditionIdentity());

  deallog.pop();
}

int
main()
{
  initlog();
  deallog << std::setprecision(3);

  deallog.push("double");
  test<double>(0);
  test<double>(1);
  test<double>(2);
  test<double>(3);
  test<double>(4);
  test<double>(5);
  deallog.pop();
  deallog.push("float");
  test<float>(0);
  test<float>(1);
  test<float>(2);
  test<float>(3);
  deallog.pop();
}
