// ---------------------------------------------------------------------
//
// Copyright (C) 2005 - 2020 by the deal.II authors
//
// This file is part of the deal.II library.
//
// The deal.II library is free software; you can use it, redistribute
// it, and/or modify it under the terms of the GNU Lesser General
// Public License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// The full text of the license can be found in the file LICENSE.md at
// the top level directory of deal.II.
//
// ---------------------------------------------------------------------



#include <deal.II/grid/grid_out.h>
#include <deal.II/grid/tria.h>
#include <deal.II/grid/tria_accessor.h>
#include <deal.II/grid/tria_iterator.h>

#include "../tests.h"


namespace
{
  /**
   * This file uses a different ordering for the vertices in a hex
   * cell than we usually do in deal.II. The different convention used
   * here originates in what we believed the ordering to be in UCD
   * format, until it was discovered in 2022 that UCD will interpret
   * this ordering to correspond to inverted cells -- as a
   * consequence, the UCD ordering was fixed, but the current file is
   * stuck on the old ordering.
   */
  constexpr std::array<unsigned int, 8> local_vertex_numbering{
    {0, 1, 5, 4, 2, 3, 7, 6}};

  /**
   * And now also in the opposite direction.
   */
  void
  reorder_old_to_new_style(std::vector<CellData<3>> &cells)
  {
    // undo the ordering above
    unsigned int tmp[GeometryInfo<3>::vertices_per_cell];
    for (auto &cell : cells)
      {
        for (const unsigned int i : GeometryInfo<3>::vertex_indices())
          tmp[i] = cell.vertices[i];
        for (const unsigned int i : GeometryInfo<3>::vertex_indices())
          cell.vertices[local_vertex_numbering[i]] = tmp[i];
      }
  }

  /**
   * Following is a set of functions that reorder the data from the
   * "current" to the "classic" format of vertex numbering of cells
   * and faces. These functions do the reordering of their arguments
   * in-place.
   */
  void
  reorder_old_to_new_style(std::vector<CellData<2>> &cells)
  {
    for (auto &cell : cells)
      std::swap(cell.vertices[2], cell.vertices[3]);
  }
} // namespace


static unsigned subcells[6][4] = {{0, 1, 2, 3},
                                  {4, 5, 6, 7},
                                  {0, 1, 5, 4},
                                  {1, 5, 6, 2},
                                  {3, 2, 6, 7},
                                  {0, 4, 7, 3}};



template <int dim>
void
test()
{
  Assert(dim == 2 || dim == 3, ExcNotImplemented());

  std::vector<Point<dim>> vertices(GeometryInfo<dim>::vertices_per_cell);
  vertices[0](0) = 0;
  vertices[0](1) = 0;
  vertices[1](0) = 2;
  vertices[1](1) = 1;
  vertices[2](0) = 3;
  vertices[2](1) = 3;
  vertices[3](0) = 0;
  vertices[3](1) = 1;
  if (dim == 3)
    {
      // for the new numbering
      //       for (unsigned int i=0; i<4; ++i)
      //  {
      //    vertices[i+4]=vertices[i];
      //    vertices[i+4](2)=1;
      //  }
      // for the old numbering
      for (unsigned int i = 0; i < 4; ++i)
        {
          std::swap(vertices[i](1), vertices[i](2));
          vertices[i + 4]    = vertices[i];
          vertices[i + 4](1) = 1;
        }
    }

  std::vector<CellData<dim>> cells(1);
  for (const unsigned int i : GeometryInfo<dim>::vertex_indices())
    cells[0].vertices[i] = i;
  cells[0].material_id = 0;

  SubCellData subcelldata;
  if (dim == 2)
    {
      subcelldata.boundary_lines.resize(GeometryInfo<dim>::faces_per_cell);
      for (const unsigned int i : GeometryInfo<dim>::face_indices())
        {
          subcelldata.boundary_lines[i].vertices[0] = i;
          subcelldata.boundary_lines[i].vertices[1] = (i + 1) % 4;
          subcelldata.boundary_lines[i].material_id = 10 * i + 1;
        }
    }
  else if (dim == 3)
    {
      subcelldata.boundary_quads.resize(GeometryInfo<dim>::faces_per_cell);
      for (const unsigned int f : GeometryInfo<dim>::face_indices())
        {
          for (unsigned int i = 0; i < GeometryInfo<dim>::vertices_per_face;
               ++i)
            subcelldata.boundary_quads[f].vertices[i] = subcells[f][i];
          subcelldata.boundary_quads[f].material_id = 10 * f + 1;
        }
    }

  reorder_old_to_new_style(cells);
  Triangulation<dim> tria;
  tria.create_triangulation(vertices, cells, subcelldata);

  GridOutFlags::Ucd ucd_flags(true, true);
  GridOut           grid_out;
  grid_out.set_flags(ucd_flags);
  grid_out.write_ucd(tria, deallog.get_file_stream());

  //   std::ofstream gnuplot_file("subcelldata.gnuplot");
  //   grid_out.write_gnuplot(tria, gnuplot_file);
  //   std::ofstream ucd_file("subcelldata.inp");
  //   grid_out.write_ucd(tria, ucd_file);
}


int
main()
{
  initlog();

  test<2>();
  test<3>();

  return 0;
}
