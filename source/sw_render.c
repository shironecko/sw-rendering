//******************** Config ********************//

#ifndef SWR_ASSERT
#define SWR_ASSERT SDL_assert
#endif // #ifndef SWR_ASSERT

#define SWR_FN LOCAL

//******************** Data ********************//

typedef struct {
	union {
		u8 e[4];

		struct {
			u8 r, g, b, a;
		};
	};
} col4;

enum {
	SWR_RC_NAME_LEN = 32 + 1,
};

typedef struct {
	col4 *texels;
	u32 width;
	u32 height;

	char name[SWR_RC_NAME_LEN];
} tex2d;

typedef struct {
	tex2d *diffuse;
	tex2d *bump;
	tex2d *specular;

	char name[SWR_RC_NAME_LEN];
} material;

typedef struct {
	u32 v[3];
	u32 uv[3];
	u32 n[3];
} face;

typedef struct {
	face *faces;
	u32 nfaces;
	material *material;
} face_group;

typedef struct {
	vec3 *vertices;
	u32 nvertices;

	vec2 *uvs;
	u32 nuvs;

	vec3 *normales;
	u32 nnormales;

	face_group *face_groups;
	u32 nface_groups;

	material *materials;
	u32 nmaterials;

	tex2d *textures;
	u32 ntextures;

	char name[SWR_RC_NAME_LEN];
} model;

typedef struct {
	tex2d *texture;
	float *z_buffer;
} swr_render_target;

enum {
	SRM_BACKFACE_CULLING = 1 << 0,
	SRM_SHADED = 1 << 1,
	SRM_TEXTURED = 1 << 2,
	SRM_WIREFRAME = 1 << 3,
};

//******************** Functions ********************//

col4 *sample_t2d(tex2d tex, u32 x, u32 y) {
	SWR_ASSERT(x < tex.width);
	SWR_ASSERT(y < tex.height);
	return tex.texels + y * tex.width + x;
}

SWR_FN void swr_line(s32 x1, s32 y1, s32 x2, s32 y2, col4 color, tex2d texture) {
	if (abs(x2 - x1) > abs(y2 - y1)) // horizontal line
	{
		if (x1 > x2) {
			s32 t = x1;
			x1 = x2;
			x2 = t;

			t = y1;
			y1 = y2;
			y2 = t;
		}

		float y = (float)y1;
		float step = (float)(y2 - y1) / (float)(x2 - x1);
		for (s32 x = x1; x <= x2; ++x) {
			*sample_t2d(texture, x, (u32)y) = color;
			y += step;
		}
	} else // vertical line
	{
		if (y1 > y2) {
			s32 t = y1;
			y1 = y2;
			y2 = t;

			t = x1;
			x1 = x2;
			x2 = t;
		}

		float x = (float)x1;
		float step = (float)(x2 - x1) / (float)(y2 - y1);
		for (s32 y = y1; y <= y2; ++y) {
			*sample_t2d(texture, (u32)x, y) = color;
			x += step;
		}
	}
}

SWR_FN void swr_clear_rt(swr_render_target *rt, col4 clear_col) {
	tex2d target_tex = *rt->texture;
	float *z_buffer = rt->z_buffer;

	for (u32 y = 0; y < target_tex.height; ++y) {
		for (u32 x = 0; x < target_tex.width; ++x) {
			*sample_t2d(target_tex, x, y) = clear_col;
		}
	}

	float infinity = 100.0f;
	for (u32 i = 0, n = target_tex.width * target_tex.height; i < n; ++i)
		z_buffer[i] = infinity;
}

SWR_FN void swr_render_model(swr_render_target *target, u32 render_mode, model *model, vec3 cam_pos,
                             mat4 model_mat, mat4 viewproj_mat, mat4 screen_mat, vec3 sun_direction,
                             col4 sun_col, float ambient_intencity, mem_pool *pool) {
	tex2d target_tex = *target->texture;
	float *z_buffer = target->z_buffer;

	u8 *old_hi_ptr = pool->hi;
	vec4 *vertices = (vec4 *)mem_push_back(pool, model->nvertices * sizeof(*vertices));
	vec3 *cam_directions = (vec3 *)mem_push_back(pool, model->nvertices * sizeof(*cam_directions));
	for (u32 i = 0, e = model->nvertices; i < e; ++i) {
		vec3 v = mul_m4v4(model_mat, v3_to_v4(model->vertices[i], 1.0f)).xyz;
		cam_directions[i] = norm_v3(sub_v3(v, cam_pos));
		vertices[i] = mul_m4v4(viewproj_mat, v3_to_v4(v, 1.0f));
	}

	face **culled_faces = (face **)mem_push_back(pool, model->nface_groups * sizeof(*culled_faces));
	u32 *nculled_faces = (u32 *)mem_push_back(pool, model->nface_groups * sizeof(*nculled_faces));

	for (u32 face_group = 0; face_group < model->nface_groups; ++face_group) {
		face *src_faces = model->face_groups[face_group].faces;
		u32 nsrc_faces = model->face_groups[face_group].nfaces;
		face *faces = culled_faces[face_group] =
		    (face *)mem_push_back(pool, nsrc_faces * sizeof(*faces));
		u32 nfaces = 0;
		for (u32 i = 0; i < nsrc_faces; ++i) {
			face face = src_faces[i];

			b32 inside_frustrum = true;
			for (u32 j = 0; j < 3; ++j) {
				vec4 vertex = vertices[face.v[j]];

				if (vertex.x > vertex.w || vertex.x < -vertex.w || vertex.y > vertex.w ||
				    vertex.y < -vertex.w || vertex.z > vertex.w || vertex.z < -vertex.w ||
				    vertex.w == 0) {
					inside_frustrum = false;
					break;
				}
			}

			if (inside_frustrum)
				faces[nfaces++] = face;
		}

		nculled_faces[face_group] = nfaces;
	}

	// TODO: avoid computing irrelevant data (?)
	for (u32 i = 0; i < model->nvertices; ++i) {
		vec4 vertex = vertices[i];
		vertex = div_v4f(vertex, vertex.w);
		vertex = mul_m4v4(screen_mat, vertex);
		vertices[i] = vertex;
	}

	for (u32 face_group = 0; face_group < model->nface_groups; ++face_group) {
		face *faces = culled_faces[face_group];
		u32 nfaces = nculled_faces[face_group];

		material *material = model->face_groups[face_group].material;
		for (u32 i = 0; i < nfaces; ++i) {
			face face = faces[i];
			vec4 verts[] = {vertices[face.v[0]], vertices[face.v[1]], vertices[face.v[2]]};

			u32 x1 = (u32)verts[0].x;
			u32 y1 = (u32)verts[0].y;
			u32 x2 = (u32)verts[1].x;
			u32 y2 = (u32)verts[1].y;
			u32 x3 = (u32)verts[2].x;
			u32 y3 = (u32)verts[2].y;

			if (render_mode & (SRM_SHADED | SRM_TEXTURED)) {
				u32 minX = minu(x1, minu(x2, x3));
				u32 minY = minu(y1, minu(y2, y3));
				u32 maxX = maxu(x1, maxu(x2, x3)) + 1;
				u32 maxY = maxu(y1, maxu(y2, y3)) + 1;

				vec3 norms[3];
				float lum[3];
				if (render_mode & SRM_SHADED) {
					// TODO: apply reverse transformations to normales
					norms[0] = model->normales[face.n[0]];
					norms[1] = model->normales[face.n[1]];
					norms[2] = model->normales[face.n[2]];

					float diffuse[3];
					for (u32 j = 0; j < 3; ++j)
						diffuse[j] = clamp(dot_v3(norms[j], sun_direction), 0, 1.0f);

					vec3 L = neg_v3(sun_direction);
					float specular[3] = {0};
					for (u32 j = 0; j < 3; ++j) {
						if (diffuse[j]) {
							vec3 V = cam_directions[face.v[j]];
							vec3 H = norm_v3(add_v3(V, L));
							specular[j] = (float)pow(dot_v3(H, norms[j]), 32);
						}
					}

					for (u32 j = 0; j < 3; ++j)
						lum[j] = ambient_intencity + diffuse[j] + specular[j];
				}

				vec2 face_uvs[3];
				face_uvs[0] = model->uvs[face.uv[0]];
				face_uvs[1] = model->uvs[face.uv[1]];
				face_uvs[2] = model->uvs[face.uv[2]];

				vec2 a = {(float)x1, (float)y1};
				vec2 b = {(float)x2, (float)y2};
				vec2 c = {(float)x3, (float)y3};

				vec2 v0 = sub_v2(b, a);
				vec2 v1 = sub_v2(c, a);

				for (u32 x = minX; x < maxX; ++x) {
					for (u32 y = minY; y < maxY; ++y) {
						// calculate barycentric coords...
						vec2 p = {(float)x, (float)y};
						vec2 v2 = sub_v2(p, a);

						float d00 = dot_v2(v0, v0);
						float d01 = dot_v2(v0, v1);
						float d11 = dot_v2(v1, v1);
						float d20 = dot_v2(v2, v0);
						float d21 = dot_v2(v2, v1);

						float denom = d00 * d11 - d01 * d01;

						float v = (d11 * d20 - d01 * d21) / denom;
						float w = (d00 * d21 - d01 * d20) / denom;
						float u = 1.0f - v - w;

						if (!(v >= -0.001 && w >= -0.001 && u >= -0.001))
							continue;

						u32 z_buff_idx = y * target_tex.width + x;
						float z = verts[1].z * v + verts[2].z * w + verts[0].z * u;
						if (z_buffer[z_buff_idx] > z) {
							float l = 1.0f;

							if (render_mode & SRM_SHADED) {
								l = lum[1] * v + lum[2] * w + lum[0] * u;
							}

							col4 texel = {255, 255, 255, 255};

							if ((render_mode & SRM_TEXTURED) && material->diffuse) {
								float tu =
								    face_uvs[1].x * v + face_uvs[2].x * w + face_uvs[0].x * u;
								float tv =
								    face_uvs[1].y * v + face_uvs[2].y * w + face_uvs[0].y * u;
								tu *= material->diffuse->width;
								tv *= material->diffuse->height;
								texel = *sample_t2d(*material->diffuse, (u32)tu, (u32)tv);
							}

							col4 fragment_col = { .e[3] = 255 };
							for (u32 j = 0; j < 3; ++j) {
								float cl = sun_col.e[j] * l / 255.0f;
								fragment_col.e[j] = (u8)clamp(texel.e[j] * cl, 0, 255.0f);
							}
							*sample_t2d(target_tex, x, y) = fragment_col;
							z_buffer[z_buff_idx] = z;
						}
					}
				}
			}
		}

		if (render_mode & SRM_WIREFRAME) {
			for (u32 i = 0; i < nfaces; ++i) {
				const col4 model_col = {255, 255, 255, 255};

				vec4 v1 = vertices[faces[i].v[0]];
				vec4 v2 = vertices[faces[i].v[1]];
				vec4 v3 = vertices[faces[i].v[2]];

				s32 x1 = (s32)v1.x;
				s32 y1 = (s32)v1.y;
				s32 x2 = (s32)v2.x;
				s32 y2 = (s32)v2.y;
				s32 x3 = (s32)v3.x;
				s32 y3 = (s32)v3.y;

				swr_line(x1, y1, x2, y2, model_col, target_tex);
				swr_line(x2, y2, x3, y3, model_col, target_tex);
				swr_line(x3, y3, x1, y1, model_col, target_tex);
			}
		}
	}

	pool->hi = old_hi_ptr;
}
