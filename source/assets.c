#ifndef AS_ASSERT
#define AS_ASSERT SDL_assert
#endif // #ifndef AS_ASSERT

// TODO: this is kinda lame, maybe there is smth I can do about it... or who cares
#define PATH_LEN 1024 + 1

#pragma pack(push, 1)
typedef struct bmp_header {
	u16 type;
	u32 size;
	u16 reserved1;
	u16 reserved2;
	u32 off_bytes;
} bmp_header;

typedef struct bmp_info_header {
	u32 size;
	s32 width;
	s32 height;
	u16 planes;
	u16 nbits;
	u32 compression;
	u32 size_image;
	s32 pels_per_meter_x;
	s32 pels_per_meter_y;
	u32 clr_used;
	u32 clr_important;
} bmp_info_header;
#pragma pack(pop)

typedef struct {
	union {
		u8 components[4];

		struct {
			u8 a; // or padding, mostly just padding
			u8 b;
			u8 g;
			u8 r;
		};
	};
} bmp_col4;

u64 load_file(const char* path, void* memory, u64 bytes_to_load)
{
	u64 nbytes_read = 0;
	SDL_RWops *file = SDL_RWFromFile(path, "rb");
	if (file)
	{
		nbytes_read = SDL_RWread(file, memory, 1, bytes_to_load);
		SDL_RWclose(file);
	}

	return nbytes_read;
}

u64 get_file_size(const char* path)
{
	SDL_RWops *file = SDL_RWFromFile(path, "rb");
	u64 size = SDL_RWsize(file);
	SDL_RWclose(file);

	return size;
}

tex2d load_bmp(const char *res_path, const char *bmp_name, mem_pool *pool, b32 dry_run) {
	char bmp_path[PATH_LEN];
	StringCombine(bmp_path, sizeof(bmp_path), res_path, bmp_name);
	u32 file_size = (u32)get_file_size(bmp_path);
	AS_ASSERT(file_size);

	u8 *initial_hi_ptr = pool->hi;
	u8 *raw_bmp = (u8 *)mem_push_back(pool, file_size);
	load_file(bmp_path, raw_bmp, file_size);

	bmp_header *file_header = (bmp_header *)raw_bmp;
	bmp_info_header *info_header = (bmp_info_header *)(raw_bmp + sizeof(*file_header));

	u16 bmp_file_type = ((u16)'M' << 8) | 'B';
	AS_ASSERT(file_header->type == bmp_file_type);
	AS_ASSERT(info_header->nbits == 32);
	AS_ASSERT(info_header->compression == 0 || info_header->compression == 3);

	info_header->height = info_header->height >= 0 ? info_header->height : -info_header->height;

	tex2d result = {0};
	StringCopy(result.name, bmp_name, sizeof(result.name));
	result.width = info_header->width;
	result.height = info_header->height;

	bmp_col4 *raw_pixels = (bmp_col4 *)(raw_bmp + file_header->off_bytes);
	u32 texels_size = sizeof(*result.texels) * result.width * result.height;
	result.texels = (col4 *)mem_push(pool, texels_size);
	for (u32 i = 0, n = result.width * result.height; i < n; ++i) {
		bmp_col4 raw_pixel = raw_pixels[i];
		result.texels[i] = (col4){raw_pixel.r, raw_pixel.g, raw_pixel.b, raw_pixel.a};
	}

	pool->hi = initial_hi_ptr;
	return result;
}

void load_mtl(const char *res_path, const char *mtl_name, material *materials,
              u32 *in_out_nmaterials, tex2d *textures, u32 *out_ntextures, mem_pool *pool,
              b32 dry_run) {
	// TODO: figure out something less clunky
	char mtl_path[PATH_LEN];
	StringCombine(mtl_path, sizeof(mtl_path), res_path, mtl_name);
	u32 file_size = (u32)get_file_size(mtl_path);
	AS_ASSERT(file_size);

	u8 *initial_hi_ptr = pool->hi;
	char *mtl_text = (char *)mem_push_back(pool, file_size + 1);
	load_file(mtl_path, mtl_text, file_size);
	mtl_text[file_size] = 0;
	u32 nmaterials = 0;
	u32 ntextures = 0;

	if (dry_run) {
		for (char *c = mtl_text; *c; ++c) {
			if (StringBeginsWith(c, "newmtl "))
				++nmaterials;
		}

		*in_out_nmaterials = nmaterials;
	} else
		nmaterials = *in_out_nmaterials;

	s32 imaterial = -1;
	for (char *text = mtl_text; (u32)(text - mtl_text) < file_size; text += SkipLine(text)) {
		if (!dry_run && StringBeginsWith(text, "newmtl ")) {
			++imaterial;
			memset(materials + imaterial, 0, sizeof(*materials));
			text += StringLen("newmtl ");
			StringCopyPred(materials[imaterial].name, text, sizeof(materials[imaterial].name),
			               StringPredCharNotInList, (void *)"\n");
		} else if (StringBeginsWith(text, "map_Kd ")) {
			++ntextures;
			if (dry_run)
				continue;

			text += StringLen("map_Kd ");
			char bmp_name[PATH_LEN] = {0};
			StringCopyPred(bmp_name, text, sizeof(bmp_name), StringPredCharNotInList,
			               (void *)" \n");
			textures[ntextures - 1] = load_bmp(res_path, bmp_name, pool, dry_run);
			materials[imaterial].diffuse = textures + ntextures - 1;
		} else if (StringBeginsWith(text, "bump ")) {
			++ntextures;
			if (dry_run)
				continue;

			text += StringLen("bump ");
			char bmp_name[PATH_LEN] = {0};
			StringCopyPred(bmp_name, text, sizeof(bmp_name), StringPredCharNotInList,
			               (void *)" \n");
			textures[ntextures - 1] = load_bmp(res_path, bmp_name, pool, dry_run);
			materials[imaterial].bump = textures + ntextures - 1;
		} else if (StringBeginsWith(text, "map_Ks ")) {
			++ntextures;
			if (dry_run)
				continue;

			text += StringLen("map_Ks ");
			char bmp_name[PATH_LEN] = {0};
			StringCopyPred(bmp_name, text, sizeof(bmp_name), StringPredCharNotInList,
			               (void *)" \n");
			textures[ntextures - 1] = load_bmp(res_path, bmp_name, pool, dry_run);
			materials[imaterial].specular = textures + ntextures - 1;
		}
	}

	AS_ASSERT((imaterial == (s32)nmaterials - 1) || dry_run);

	AS_ASSERT(out_ntextures);
	*out_ntextures = ntextures;

	pool->hi = initial_hi_ptr;
}

u32 parse_v3(char *in_text, vec3 *out_v3) {
	char *text = in_text;
	vec3 result;

	for (u32 i = 0; i < 3; ++i) {
		text += ParseFloat(text, &result.e[i]);
		++text;
	}

	*out_v3 = result;
	return (u32)(text - in_text);
}

void load_obj(const char *res_path, const char *obj_name, mem_pool *pool, model *in_out_model,
              u32 *in_out_nfaces_total, b32 dry_run) {
	char obj_path[PATH_LEN];
	StringCombine(obj_path, sizeof(obj_path), res_path, obj_name);
	u32 file_size = (u32)get_file_size(obj_path);
	AS_ASSERT(file_size);

	u8 *initial_hi_ptr = pool->hi;
	char *obj_text = (char *)mem_push_back(pool, file_size + 1);
	load_file(obj_path, obj_text, file_size);
	obj_text[file_size] = 0;

	vec3 *vertices = 0;
	u32 nvertices = 0;

	vec2 *uvs = 0;
	u32 nuvs = 0;

	vec3 *normales = 0;
	u32 nnormales = 0;

	face_group *face_groups = 0;
	u32 nface_groups = 0;
	face *faces = 0;
	u32 nfaces_total = 0;

	material *materials = 0;
	u32 nmaterials = 0;

	tex2d *textures = 0;
	u32 ntextures = 0;

	if (!dry_run) {
		vertices = (vec3 *)mem_push(pool, in_out_model->nvertices * sizeof(*vertices));
		uvs = (vec2 *)mem_push(pool, in_out_model->nuvs * sizeof(*uvs));
		normales = (vec3 *)mem_push(pool, in_out_model->nnormales * sizeof(*normales));
		face_groups =
		    (face_group *)mem_push(pool, in_out_model->nface_groups * sizeof(*face_groups));
		faces = (face *)mem_push(pool, *in_out_nfaces_total * sizeof(*faces));
		materials = (material *)mem_push(pool, in_out_model->nmaterials * sizeof(*materials));
		textures = (tex2d *)mem_push(pool, in_out_model->ntextures * sizeof(*textures));

		nmaterials = in_out_model->nmaterials;
	}

	for (char *text = obj_text; (u32)(text - obj_text) < file_size; text += SkipLine(text)) {
		if (StringBeginsWith(text, "mtllib ")) {
			// NOTE: no support for multiple mtl filse yet
			char *local_text = text + 7;
			char mtl_name[PATH_LEN];
			StringCopyPred(mtl_name, local_text, sizeof(mtl_name), StringPredCharNotInList,
			               (void *)"\n");
			u32 tc = 0;
			load_mtl(res_path, mtl_name, materials, &nmaterials, textures + ntextures, &tc, pool,
			         dry_run);
			ntextures += tc;
		} else if (StringBeginsWith(text, "usemtl ")) {
			// use material group
			++nface_groups;
			if (dry_run)
				continue;

			char *local_text = text + StringLen("usemtl ");
			char material_name[SWR_RC_NAME_LEN];
			StringCopyPred(material_name, local_text, sizeof(material_name),
			               StringPredCharNotInList, (void *)" \n");
			s32 imaterial = -1;
			for (u32 i = 0; i < nmaterials; ++i) {
				if (StringCompare(material_name, materials[i].name)) {
					imaterial = i;
					break;
				}
			}

			AS_ASSERT(imaterial != -1);
			face_groups[nface_groups - 1].faces = faces + nfaces_total;
			face_groups[nface_groups - 1].nfaces = 0;
			face_groups[nface_groups - 1].material = materials + imaterial;
		} else if (StringBeginsWith(text, "f ")) {
			// face
			if (!nface_groups) {
				++nface_groups;
				if (!dry_run) {
					face_groups->faces = faces;
					face_groups->nfaces = 0;
					face_groups->material = 0;
				}
			}

			face face;
			char *local_text = text + 2;
			for (u32 i = 0; i < 3; ++i) {
				local_text += ParseUInteger(local_text, &face.v[i]) + 1;
				--face.v[i];

				local_text += ParseUInteger(local_text, &face.uv[i]) + 1;
				--face.uv[i];

				local_text += ParseUInteger(local_text, &face.n[i]) + 1;
				--face.n[i];
			}

			if (!dry_run) {
				faces[nfaces_total] = face;
				++face_groups[nface_groups - 1].nfaces;
			}

			++nfaces_total;

			if (IsNumber(*local_text)) {
				// triangulate a quad
				face.v[1] = face.v[2];
				face.uv[1] = face.uv[2];
				face.n[1] = face.n[2];
				local_text += ParseUInteger(local_text, &face.v[2]) + 1;
				--face.v[2];
				local_text += ParseUInteger(local_text, &face.uv[2]) + 1;
				--face.uv[2];
				local_text += ParseUInteger(local_text, &face.n[2]) + 1;
				--face.n[2];

				if (!dry_run) {
					faces[nfaces_total] = face;
					++face_groups[nface_groups - 1].nfaces;
				}

				++nfaces_total;
			}
		} else if (StringBeginsWith(text, "v ")) {
			// vertex
			++nvertices;
			if (dry_run)
				continue;

			vec3 vertice;
			parse_v3(text + 2, &vertice);
			vertices[nvertices - 1] = vertice;
		} else if (StringBeginsWith(text, "vt ")) {
			// uv
			++nuvs;
			if (dry_run)
				continue;

			vec2 uv;
			char *local_text = text + 3;
			for (u32 i = 0; i < 2; ++i)
				local_text += ParseFloat(local_text, &uv.e[i]) + 1;

			uvs[nuvs - 1] = uv;
		} else if (StringBeginsWith(text, "vn ")) {
			// normal
			++nnormales;
			if (dry_run)
				continue;

			vec3 normale;
			parse_v3(text + 3, &normale);
			normales[nnormales - 1] = normale;
		}
	}

	if (dry_run) {
		in_out_model->nvertices = nvertices;
		in_out_model->nuvs = nuvs;
		in_out_model->nnormales = nnormales;
		in_out_model->nface_groups = nface_groups;
		in_out_model->nmaterials = nmaterials;
		in_out_model->ntextures = ntextures;
		*in_out_nfaces_total = nfaces_total;
	} else {
		AS_ASSERT(in_out_model->nvertices == nvertices);
		AS_ASSERT(in_out_model->nuvs == nuvs);
		AS_ASSERT(in_out_model->nnormales == nnormales);
		AS_ASSERT(in_out_model->nface_groups == nface_groups);
		AS_ASSERT(in_out_model->nmaterials == nmaterials);
		AS_ASSERT(in_out_model->ntextures == ntextures);
		AS_ASSERT(*in_out_nfaces_total == nfaces_total);

		u32 nfaces_in_groups = 0;
		for (u32 i = 0; i < nface_groups; ++i)
			nfaces_in_groups += face_groups[i].nfaces;

		AS_ASSERT(nfaces_in_groups == nfaces_total);

		in_out_model->vertices = vertices;
		in_out_model->uvs = uvs;
		in_out_model->normales = normales;
		in_out_model->face_groups = face_groups;
		in_out_model->materials = materials;
		in_out_model->textures = textures;
	}

	pool->hi = initial_hi_ptr;
}

void load_model(const char *res_path, const char *obj_name, mem_pool *pool, model *out_model) {
	AS_ASSERT(res_path);
	AS_ASSERT(obj_name);
	AS_ASSERT(pool);
	AS_ASSERT(out_model);

	// a dry run to collect the number of vertices, size of the textures etc.
	mem_pool initial_pool = *pool;
	u32 nfaces = 0;
	load_obj(res_path, obj_name, pool, out_model, &nfaces, true);
	AS_ASSERT(!memcmp(&initial_pool, pool, sizeof(initial_pool)));

	// an actual process of loading a model
	load_obj(res_path, obj_name, pool, out_model, &nfaces, false);
	AS_ASSERT(initial_pool.hi == pool->hi);
}
