/*
   This file is part of bpvo.

   bpvo is free software: you can redistribute it and/or modify
   it under the terms of the Lesser GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   bpvo is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   Lesser GNU General Public License for more details.

   You should have received a copy of the Lesser GNU General Public License
   along with bpvo.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 * Contributor: halismai@cs.cmu.edu
 */

#include "bpvo/point_cloud.h"
#include "bpvo/debug.h"

#include <cstring>
#include <iostream>
#include <fstream>

namespace bpvo {

PointWithInfo::PointWithInfo() : _w(0.0f) {}

PointWithInfo::PointWithInfo(const Point& xyzw, const Color& rgba, float w)
    : _xyzw(xyzw), _rgba(rgba), _w(w) {}

const Point& PointWithInfo::xyzw() const { return _xyzw; }
Point& PointWithInfo::xyzw() { return _xyzw; }

auto PointWithInfo::rgba() const -> const Color& { return _rgba; }
auto PointWithInfo::rgba() -> Color& { return _rgba; }

const float& PointWithInfo::weight() const { return _w; }
float& PointWithInfo::weight() { return _w; }

void PointWithInfo::setZero()
{
  _xyzw.setZero();
  _rgba.setZero();
  _w = 0.0f;

  memset(_pad, 0, sizeof(_pad));
}

std::ostream& operator<<(std::ostream& os, const PointWithInfo& p)
{
  os << "[" << p.xyzw().transpose() << "] "
     << "<" << p.rgba().transpose() << "> "
     << "w: " << p.weight();

  return os;
}

PointCloud::PointCloud() : _points(), _pose(Transform::Identity()) {}

PointCloud::PointCloud(const PointWithInfoVector& v) :
    PointCloud(v, Transform::Identity()) {}

PointCloud::PointCloud(const PointWithInfoVector& v, const Transform& T) :
    _points(v), _pose(T) {}

PointCloud::PointCloud(size_t n, const Transform& T)
  : _points(n), _pose(T) {}

PointCloud::PointCloud(size_t n)
  : PointCloud(n, Transform::Identity()) {}

PointCloud::~PointCloud() {}

const typename PointWithInfoVector::value_type& PointCloud::operator[](int i) const
{
  assert( i >= 0 && i < (int) size() );
  return _points[i];
}

typename PointWithInfoVector::value_type& PointCloud::operator[](int i)
{
  assert( i >= 0 && i < (int) size() );
  return _points[i];
}

const PointWithInfoVector& PointCloud::points() const
{
  return _points;
}

PointWithInfoVector& PointCloud::points()
{
  return _points;
}

void PointCloud::clear()
{
  _points.clear();
}

bool PointCloud::empty() const
{
  return _points.empty();
}

size_t PointCloud::size() const
{
  return _points.size();
}

void PointCloud::reserve(size_t n)
{
  _points.reserve(n);
}

void PointCloud::resize(size_t n)
{
  _points.resize(n);
}

auto PointCloud::pose() const -> const Transform& { return _pose; }
auto PointCloud::pose() -> Transform& { return _pose; }

static inline bool IsLittleEndain()
{
  int n = 1;
  return (static_cast<int>(*((unsigned char*)(&n))) == 1);
}

bool ToPlyFile(std::string filename, const PointWithInfoVector& points, std::string comment)
{
  try {
    std::ofstream ofs(filename);

    ofs << "ply" << std::endl;
    ofs << "format ascii 1.0" << std::endl;
    //ofs << "format binary_" << (IsLittleEndain() ? "little" : "big") << "_endian 1.0" << std::endl;
    ofs << "comment generated by bpvo" << std::endl;
    if(!comment.empty())
      ofs << "comment " << comment << std::endl;
    ofs << "element vertex " << points.size() << std::endl;
    ofs << "property float x" << std::endl;
    ofs << "property float y" << std::endl;
    ofs << "property float z" << std::endl;
    ofs << "property uchar red" << std::endl;
    ofs << "property uchar green" << std::endl;
    ofs << "property uchar blue" << std::endl;
    ofs << "property uchar alpha" << std::endl;
    ofs << "end_header" << std::endl;

    //
    // we'll copy the data into a buffer and write everything at once
    //
    int nbytes = points.size() * (3*sizeof(float) + 4*sizeof(uint8_t));
    std::vector<char> data(nbytes);
    auto ptr = data.data();
    for(const auto& p : points) {
        ofs << p.xyzw().x() << " " << p.xyzw().y() << " " << p.xyzw().z() << " "
            << (int)p.rgba().x() << " " << (int)p.rgba().y() << " " << (int)p.rgba().z() << " " << (int)p.rgba().w() << "\n";
      //memcpy(ptr, p.xyzw().data(), 3*sizeof(float));
      //ptr += 3*sizeof(float);
      //memcpy(ptr, p .rgba().data(), 4*sizeof(uint8_t));
      //ptr += 4*sizeof(uint8_t);
    }

    //ofs.write(data.data(), nbytes);

    ofs.close();
    return !ofs.bad();
  } catch(const std::exception& ex) {
    Warn("Failed to open file %s\n", filename.c_str());
    return false;
  }
}

} // bpvo

