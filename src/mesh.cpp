#include "mesh.h"


void Mesh::Render()
{
	if (_texture != 0) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(_textureType, _texture);
	}
	glBindVertexArray(_vao);
	glDrawElements(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, (GLvoid*)(0));
	glBindVertexArray(0);
}

void Mesh::EnablePerFaceShading(bool enable)
{
	if (enable != _per_face_shading) {
		_per_face_shading = enable;
		if (_per_face_shading) {
			assert(_mesh.has_face_colors());
		}
		else {

		}
	}
}


void Mesh::ComputeBoundingBox()
{
	assert(_mesh.n_vertices() > 0);
	TriMesh::VertexIter v_it, v_end(_mesh.vertices_end());
	TriMesh::Point center(0, 0, 0);
	_bbox.min = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	_bbox.max = glm::vec3(FLT_MIN, FLT_MIN, FLT_MIN);


	for (v_it = _mesh.vertices_begin(); v_it != v_end; ++v_it) {
		TriMesh::Point pos = _mesh.point(*v_it);
		center += pos;

		_bbox.min.x = fminf(_bbox.min.x, pos[0]);
		_bbox.min.y = fminf(_bbox.min.y, pos[1]);
		_bbox.min.z = fminf(_bbox.min.z, pos[2]);
		_bbox.max.x = fmaxf(_bbox.max.x, pos[0]);
		_bbox.max.y = fmaxf(_bbox.max.y, pos[1]);
		_bbox.max.z = fmaxf(_bbox.max.z, pos[2]);
	}

	center = center / _mesh.n_vertices();
	_bbox.center = glm::vec3(center[0], center[1], center[2]);

}

void Mesh::SetInitialTransformation()
{
	_transformation = glm::translate(_transformation, -_bbox.center);
	float scaleX = 2 / (_bbox.max.x - _bbox.min.x);
	float scaleY = 2 / (_bbox.max.y - _bbox.min.y);
	float scaleZ = 2 / (_bbox.max.z - _bbox.min.z);
	_transformation = glm::scale(_transformation, glm::vec3(scaleX, scaleY, scaleZ));
}

void Mesh::SetupVAO()
{
	VBOInfo info;
	PopulateVBO(info);


	GLuint index = 0;
	glGenVertexArrays(1, &_vao);
	glGenBuffers(1, &_vbo);
	glGenBuffers(1, &_ibo);

	glBindVertexArray(_vao);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, info.size, info.data.get(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(GLuint), _indices.data(), GL_STATIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(index);
	glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, info.vertex_size, (GLvoid*)0);
	index++;

	// vertex normals
	glEnableVertexAttribArray(index);
	glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, info.vertex_size, (GLvoid*)(info.normal_offset));
	index++;

	if (VertexHasColorAttrib()) {
		glEnableVertexAttribArray(index);
		glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, info.vertex_size, (GLvoid*)(info.color_offset));
		index++;
	}

	if (_mesh.has_vertex_texcoords2D()) {
		glEnableVertexAttribArray(index);
		glVertexAttribPointer(index, 2, GL_FLOAT, GL_FALSE, info.vertex_size, (GLvoid*)(info.texcoord_offset));
	}

	glBindVertexArray(0);

}

void Mesh::PopulateVBO(VBOInfo& info)
{
	CalculateVBOSize(info);
	PopulateVBOData(info);
}

void Mesh::CalculateVBOSize(VBOInfo& info)
{
	size_t vertex_size = sizeof(TriMesh::Point);

	info.normal_offset = vertex_size;
	vertex_size += sizeof(TriMesh::Normal);

	if (VertexHasColorAttrib()) {
		info.color_offset = vertex_size;
		// TriMesh::Color is a unsigned char array of 3 elements
		// we convert it into a float array 
		vertex_size += 3 * sizeof(float);
	}

	if (_mesh.has_vertex_texcoords2D()) {
		info.texcoord_offset = vertex_size;
		vertex_size += sizeof(TriMesh::TexCoord2D);
	}

	info.vertex_size = vertex_size;

	size_t total_vertices = 0;
	if (_per_face_shading) {
		for (TriMesh::ConstVertexIter v_it = _mesh.vertices_begin(); v_it != _mesh.vertices_end(); ++v_it) {
			if (_mesh.is_boundary(*v_it))
				total_vertices += _mesh.valence(*v_it) - 1;
			else
				total_vertices += _mesh.valence(*v_it);
		}
	}
	else
		total_vertices = _mesh.n_vertices();

	info.size = info.vertex_size * total_vertices;
}

void Mesh::PopulateVBOData(VBOInfo& info)
{
	_indices.resize(_mesh.n_faces() * 3);

	char* buffer = new char[info.size];

	TriMesh::VertexIter v_it, v_end(_mesh.vertices_end());
	int v_count = 0;
	std::unique_ptr<char> per_vertex_data(new char[info.vertex_size]);

	if (_per_face_shading) {
		for (v_it = _mesh.vertices_begin(); v_it != v_end; ++v_it) {
			TriMesh::VertexOHalfedgeCCWIter vh_end = _mesh.voh_ccwend(*v_it);
		
			for (TriMesh::VertexOHalfedgeCCWIter vh_it = _mesh.voh_ccwbegin(*v_it); vh_it != vh_end; ++vh_it) {
				if (_mesh.is_boundary(*vh_it))
					continue;
				PopulateVertexData(*vh_it, per_vertex_data.get(), info);
				memcpy(buffer + v_count*info.vertex_size, per_vertex_data.get(), info.vertex_size);
				PopulateIBO(*vh_it, v_count);
				v_count++;
			}
		}
	}
	else {
		for (v_it = _mesh.vertices_begin(); v_it != v_end; ++v_it) {
			PopulateVertexData(_mesh.halfedge_handle(*v_it), per_vertex_data.get(), info);
			memcpy(buffer + v_count*info.vertex_size, per_vertex_data.get(), info.vertex_size);
			v_count++;
		}

		for (auto& face : _mesh.faces()) {
			TriMesh::ConstFaceVertexCCWIter fv_beg = _mesh.cfv_ccwbegin(face), fv_it, fv_end = _mesh.cfv_ccwend(face);
			for (fv_it = fv_beg; fv_it != fv_end; ++fv_it)
				_indices[3*face.idx()+std::distance(fv_beg, fv_it)] = fv_it->idx();
		}

	}

	info.data = std::unique_ptr<char>(buffer);

}

void Mesh::PopulateVertexData(const TriMesh::HalfedgeHandle& hh, char* data, const VBOInfo& info)
{
	auto& vh = _mesh.from_vertex_handle(hh);
	TriMesh::Point pos = _mesh.point(vh);
	memcpy(data, pos.data(), sizeof(TriMesh::Point));

	TriMesh::Normal normal = _mesh.normal(vh);
	memcpy(data+info.normal_offset, normal.data(), sizeof(TriMesh::Normal));

	if (VertexHasColorAttrib()) {
		TriMesh::Color color;
		if (_per_face_shading) {
			TriMesh::FaceHandle fh;
			if (_mesh.is_boundary(hh))
				fh = _mesh.face_handle(_mesh.opposite_halfedge_handle(hh));
			else
				fh = _mesh.face_handle(hh);
			color = _mesh.color(fh);
		}
		else
			color = _mesh.color(vh);
		float f_color[3];
		for (int i = 0; i < 3; i++)
			f_color[i] = float((unsigned)color[i]) / 255.0f;
		memcpy(data+info.color_offset, f_color, sizeof(f_color));
	}

	if (_mesh.has_vertex_texcoords2D()) {
		TriMesh::TexCoord2D texcoord = _mesh.texcoord2D(vh);
		memcpy(data+info.texcoord_offset, texcoord.data(), sizeof(TriMesh::TexCoord2D));
	}
}



void Mesh::PopulateIBO(const TriMesh::HalfedgeHandle& hh, int v_count)
{
	TriMesh::FaceHandle fh = _mesh.face_handle(hh);
	TriMesh::ConstFaceHalfedgeCCWIter fh_it = _mesh.cfh_ccwbegin(fh);
	auto& hh1 = *fh_it;
	++fh_it;
	auto& hh2 = *fh_it;
	++fh_it;
	auto& hh3 = *fh_it;
	if (hh1 == hh)
		_indices[fh.idx() * 3] = v_count;
	else if (hh2 == hh)
		_indices[fh.idx() * 3 + 1] = v_count;
	else if (hh3 == hh)
		_indices[fh.idx() * 3 + 2] = v_count;
	else
		assert(0);
}

bool Mesh::VertexHasColorAttrib()
{
	return ((_per_face_shading && _mesh.has_face_colors()) || _mesh.has_vertex_colors());
}

void Mesh::DeleteIsolatedVerts()
{
	TriMesh::VertexIter cv_it;
	for (cv_it = _mesh.vertices_begin(); cv_it != _mesh.vertices_end(); ++cv_it) {
		if (_mesh.valence(*cv_it) == 0)
			_mesh.delete_vertex(*cv_it, true);
	}

	_mesh.garbage_collection();
}

