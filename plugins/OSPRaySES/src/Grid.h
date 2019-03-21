#ifndef _SES_GRID_H_
#define _SES_GRID_H_

#include <vector>
#include <algorithm>
#include <stdio.h>

#include "Common.h"

namespace megamol {
  namespace ospray {
    namespace ses {

      template <typename T> struct GridCell {
        T data;
      };

      template <typename T> struct Grid {
        vec3d origin;
        vec3d cellLength;
        vec3i resolution;
        double diagonalLength;
        std::vector<GridCell<T>> cells;
      };

      template <typename T> struct GridNeighboorhoodIterator {
        int currentObjectIdx;
        vec3i min;
        vec3i max;
        vec3i currentCell;
        Grid<std::vector<T>> *grid;
        GridCell<std::vector<T>> *cell;
      };

      template <typename T> vec3i Grid_discretize(Grid<T> &grid, const vec3d &p) {
        vec3i cell;
        const vec3d c = p - grid.origin;
        cell.x = static_cast<int>(c.x / grid.cellLength.x);
        cell.y = static_cast<int>(c.y / grid.cellLength.y);
        cell.z = static_cast<int>(c.z / grid.cellLength.z);
        return cell;
      }

      template <typename T> void Grid_init(Grid<T> &grid, const vec3d &origin, const vec3d &cellLength, const vec3i &resolution) {
        grid.origin = origin;
        grid.resolution = resolution;
        grid.cellLength = cellLength;
        grid.diagonalLength = length(grid.cellLength);
        grid.cells = std::vector<GridCell<T>>(grid.resolution.x*grid.resolution.y*grid.resolution.z, GridCell<T>());
      }

      template <typename T> void GridNeighboorhoodIterator_init(GridNeighboorhoodIterator<T> &iterator, Grid<std::vector<T>> *grid, const vec3d &p) {
        iterator.grid = grid;
        iterator.cell = nullptr;
        const vec3i cell = Grid_discretize<std::vector<T>>(*grid, p);
        iterator.min.x = std::max(0, cell.x - 1);
        iterator.min.y = std::max(0, cell.y - 1);
        iterator.min.z = std::max(0, cell.z - 1);

        iterator.max.x = std::min(grid->resolution.x, cell.x + 2);
        iterator.max.y = std::min(grid->resolution.y, cell.y + 2);
        iterator.max.z = std::min(grid->resolution.z, cell.z + 2);

        iterator.currentCell = iterator.min;
        iterator.currentObjectIdx = 0;
      }

      template <typename T> int Grid_position_to_index(const Grid<T> &grid, const vec3i &p) {
        return grid.resolution.x*grid.resolution.y*p.z + grid.resolution.x*p.y + p.x;
      }

      template <typename T> void Grid_put(Grid<std::vector<T>> &grid, const vec3d &p, const T &t) {
        const vec3i cell = Grid_discretize<std::vector<T>>(grid, p);
        
        const int idx = Grid_position_to_index<std::vector<T>>(grid, cell);
        grid.cells[idx].data.push_back(t);
      }

      template <typename T> void Grid_set(Grid<T> &grid, const vec3d &p, const T &t) {
        const vec3i cell = Grid_discretize<T>(grid, p);

        const int idx = Grid_position_to_index<T>(grid, cell);
        grid.cells[idx].data = t;
      }

      template <typename T> bool GridNeighboorhoodIterator_next(GridNeighboorhoodIterator<T> &iterator, T &t) {

        if (iterator.cell == nullptr) {
          const int idx = Grid_position_to_index<std::vector<T>>(*iterator.grid, iterator.currentCell);
          iterator.cell = &iterator.grid->cells[idx];
          iterator.currentObjectIdx = 0;
        }

        while (iterator.cell->data.size() <= iterator.currentObjectIdx) {
          iterator.currentCell.x = iterator.currentCell.x + 1;
          // find new position if possible
          if (iterator.currentCell.x >= iterator.max.x) {
            iterator.currentCell.x = iterator.min.x;
            iterator.currentCell.y = iterator.currentCell.y + 1;

            if (iterator.currentCell.y >= iterator.max.y) {
              iterator.currentCell.y = iterator.min.y;
              iterator.currentCell.z = iterator.currentCell.z + 1;
              if (iterator.currentCell.z >= iterator.max.z) {
                return false;
              }

            }

          }

          iterator.cell = &iterator.grid->cells[Grid_position_to_index<std::vector<T>>(*iterator.grid, iterator.currentCell)];
          iterator.currentObjectIdx = 0;
        }

        t = iterator.cell->data[iterator.currentObjectIdx++];
        return true;
      }
    }
  }
}

#endif
