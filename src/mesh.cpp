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
	PopulateIBO();

	glGenVertexArrays(1, &_vao);
	glGenBuffers(1, &_vbo);
	glGenBuffers(1, &_ibo);

	glBindVertexArray(_vao);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, info.size, info.data.get(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(GLuint), _indices.data(), GL_STATIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, info.vertex_size, (GLvoid*)0);

	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, info.vertex_size, (GLvoid*)(info.normal_offset));

	if (_mesh.has_vertex_texcoords2D()) {
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, info.vertex_size, (GLvoid*)(info.texcoord_offset));
	}

	if (_mesh.has_vertex_colors()) {
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_UNSIGNED_BYTE, GL_FALSE, info.vertex_size, (GLvoid*)(info.color_offset));
	}

	glBindVertexArray(0);

}

void Mesh::PopulateVBO(VBOInfo& info)
{
	size_t vertex_size = sizeof(TriMesh::Point);

	assert(_mesh.has_vertex_normals());
	info.normal_offset = vertex_size;
	vertex_size += sizeof(TriMesh::Normal);
	
	if (_mesh.has_vertex_texcoords2D()) {
		info.texcoord_offset = vertex_size;
		vertex_size += sizeof(TriMesh::TexCoord2D);
	}

	if (_mesh.has_vertex_colors()) {
		info.color_offset = vertex_size;
		vertex_size += sizeof(TriMesh::Color);
	}
	info.vertex_size = vertex_size;
	info.size = vertex_size * _mesh.n_vertices();

	char* vertex_data = new char[info.size];

	TriMesh::VertexIter v_it, v_end(_mesh.vertices_end());
	int v_count = 0;
	for (v_it = _mesh.vertices_begin(); v_it != v_end; ++v_it) {
		TriMesh::Point pos = _mesh.point(*v_it);
		memcpy(vertex_data + v_count*vertex_size, pos.data(), sizeof(TriMesh::Point));
		
		TriMesh::Normal normal = _mesh.normal(*v_it);
		memcpy(vertex_data + v_count*vertex_size + info.normal_offset, normal.data(), sizeof(TriMesh::Normal));
		
		if (_mesh.has_vertex_texcoords2D()) {
			TriMesh::TexCoord2D texcoord = _mesh.texcoord2D(*v_it);
			memcpy(vertex_data + v_count*vertex_size + info.texcoord_offset, texcoord.data(), sizeof(TriMesh::TexCoord2D));
		}
		if (_mesh.has_vertex_colors()) {
			TriMesh::Color color = _mesh.color(*v_it);
			memcpy(vertex_data + v_count*vertex_size + info.color_offset, color.data(), sizeof(TriMesh::Color));
		}
		v_count++;
	}

	info.data = std::unique_ptr<char>(vertex_data);
}

void Mesh::PopulateIBO()
{
	for (auto& face : _mesh.faces()) {
		TriMesh::ConstFaceVertexCCWIter fv_it, fv_end = _mesh.cfv_ccwend(face);
		for (fv_it = _mesh.cfv_ccwbegin(face); fv_it != fv_end; ++fv_it)
			_indices.push_back(fv_it->idx());
	}
}