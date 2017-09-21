#ifndef MESH_H_
#define MESH_H_

#include "common.h"
#include "geometry.h"

#include <OpenMesh\Core\IO\MeshIO.hh>
#include <OpenMesh\Core\Mesh\TriMesh_ArrayKernelT.hh>

using TriMesh = OpenMesh::TriMesh_ArrayKernelT<>;

class Mesh : public Geometry {
    friend class ResourceManager;

    struct VBOInfo {
        std::unique_ptr<char> data;
        size_t                size;
        size_t                vertex_size;
        size_t                normal_offset;
        size_t                texcoord_offset;
        size_t                color_offset;
    };

public:
    Mesh() : _per_face_shading(false) {}

    TriMesh&         GetMeshObj()                       { return _mesh; }
    void             EnablePerFaceShading(bool enable);
    virtual void     Render();

private:
    void     ComputeBoundingBox();
    void     SetInitialTransformation();
    void     SetupVAO();
    void     PopulateVBO(VBOInfo&);
    void     CalculateVBOSize(VBOInfo&);
    void     PopulateVBOData(VBOInfo&);
    void     PopulateVertexData(const TriMesh::HalfedgeHandle& hh, char* data, const VBOInfo&);
    void     PopulateIBO(const TriMesh::HalfedgeHandle&, int);
    bool     VertexHasColorAttrib();
    void     DeleteIsolatedVerts();

private:
    TriMesh             _mesh;
    std::vector<GLuint> _indices;
    bool                _per_face_shading;
};


#endif
