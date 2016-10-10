// TODO: this is kinda lame, maybe there is smth I can do about it... or who cares
#define PATH_LEN 1024 + 1

#pragma pack(push, 1)
typedef struct bmp_header {
	u16 type;
	u32 size;
	u16 reserved1;
	u16 reserved2;
	u32 offBytes;
} bmp_header;

typedef struct bmp_info_header {
	u32 size;
	s32 width;
	s32 height;
	u16 planes;
	u16 bitCount;
	u32 compression;
	u32 sizeImage;
	s32 xPelsPerMeter;
	s32 yPelsPerMeter;
	u32 clrUsed;
	u32 clrImportant;
} bmp_info_header;
#pragma pack(pop)

struct bmp_col4 {
	union {
		u8 components[4];

		struct {
			u8 a; // or padding, mostly just padding
			u8 b;
			u8 g;
			u8 r;
		};
	};
};

tex2d LoadBmp(const char *resourcePath, const char *bmpName, mem_pool *pool, b32 dryRun) {
	char bmpPath[PATH_LEN];
	StringCombine(bmpPath, sizeof(bmpPath), resourcePath, bmpName);
	u32 fileSize = (u32)PlatformGetFileSize(bmpPath);
	assert(fileSize);

	u8 *initialHiPtr = pool->hiPtr;
	u8 *rawBitmap = (u8 *)mem_push_back(pool, fileSize);
	PlatformLoadFile(bmpPath, rawBitmap, fileSize);

	bmp_header *fileHeader = (bmp_header *)rawBitmap;
	bmp_info_header *infoHeader = (bmp_info_header *)(rawBitmap + sizeof(fileHeader));

	u16 bitmapFileType = (u16('M') << 8) | 'B';
	assert(fileHeader->type == bitmapFileType);
	assert(infoHeader->bitCount == 32);
	assert(infoHeader->compression == 0 || infoHeader->compression == 3);

	infoHeader->height = infoHeader->height >= 0 ? infoHeader->height : -infoHeader->height;

	tex2d result = {0};
	StringCopy(result.name, bmpName, sizeof(result.name));
	result.width = infoHeader->width;
	result.height = infoHeader->height;

	bmp_col4 *rawPixels = (bmp_col4 *)(rawBitmap + fileHeader->offBytes);
	u32 texelsSize = sizeof(*result.texels) * result.width * result.height;
	result.texels = (col4 *)mem_push(pool, texelsSize);
	for (u32 i = 0, n = result.width * result.height; i < n; ++i) {
		bmp_col4 rawPixel = rawPixels[i];
        result.texels[i] = (col4){rawPixel.r, rawPixel.g, rawPixel.b, rawPixel.a};
	}

	pool->hiPtr = initialHiPtr;
	return result;
}

/*
 * Returns number of materials loaded
 */
void LoadMtl(const char *resourcePath, const char *mtlName, material *materials,
             u32 *inOutMaterialsCount, tex2d *textures, u32 *outTexturesCount, mem_pool *pool,
             b32 dryRun) {
	// TODO: figure out something less clunky
	char mtlPath[PATH_LEN];
	StringCombine(mtlPath, sizeof(mtlPath), resourcePath, mtlName);
	u32 fileSize = (u32)PlatformGetFileSize(mtlPath);
	assert(fileSize);

	u8 *initialHiPtr = pool->hiPtr;
	char *mtlText = (char *)mem_push_back(pool, fileSize + 1);
	PlatformLoadFile(mtlPath, mtlText, fileSize);
	mtlText[fileSize] = 0;
	u32 matCount = 0;
	u32 ntextures = 0;

	if (dryRun) {
		for (char *c = mtlText; *c; ++c) {
			if (StringBeginsWith(c, "newmtl ")) ++matCount;
		}

		*inOutMaterialsCount = matCount;
	} else
		matCount = *inOutMaterialsCount;

	s32 materialIdx = -1;
	for (char *text = mtlText; u32(text - mtlText) < fileSize; text += SkipLine(text)) {
		if (!dryRun && StringBeginsWith(text, "newmtl ")) {
			++materialIdx;
			MemorySet(materials + materialIdx, 0, sizeof(*materials));
			text += StringLen("newmtl ");
			StringCopyPred(materials[materialIdx].name, text, sizeof(materials[materialIdx].name),
			               StringPredCharNotInList, (void *)"\n");
		} else if (StringBeginsWith(text, "map_Kd ")) {
			++ntextures;
			if (dryRun) continue;

			text += StringLen("map_Kd ");
			char bmpName[PATH_LEN] = {0};
			StringCopyPred(bmpName, text, sizeof(bmpName), StringPredCharNotInList, (void *)" \n");
			textures[ntextures - 1] = LoadBmp(resourcePath, bmpName, pool, dryRun);
			materials[materialIdx].diffuse = textures + ntextures - 1;
		} else if (StringBeginsWith(text, "bump ")) {
			++ntextures;
			if (dryRun) continue;

			text += StringLen("bump ");
			char bmpName[PATH_LEN] = {0};
			StringCopyPred(bmpName, text, sizeof(bmpName), StringPredCharNotInList, (void *)" \n");
			textures[ntextures - 1] = LoadBmp(resourcePath, bmpName, pool, dryRun);
			materials[materialIdx].bump = textures + ntextures - 1;
		} else if (StringBeginsWith(text, "map_Ks ")) {
			++ntextures;
			if (dryRun) continue;

			text += StringLen("map_Ks ");
			char bmpName[PATH_LEN] = {0};
			StringCopyPred(bmpName, text, sizeof(bmpName), StringPredCharNotInList, (void *)" \n");
			textures[ntextures - 1] = LoadBmp(resourcePath, bmpName, pool, dryRun);
			materials[materialIdx].specular = textures + ntextures - 1;
		}
	}

	assert((materialIdx == (s32)matCount - 1) || dryRun);

	assert(outTexturesCount);
	*outTexturesCount = ntextures;

	pool->hiPtr = initialHiPtr;
}

u32 ParseVector3(char *inText, vec4 *outVector) {
	char *text = inText;
	vec4 result;

	for (u32 i = 0; i < 3; ++i) {
		text += ParseFloat(text, &result.components[i]);
		++text;
	}

	*outVector = result;
	return text - inText;
}

void LoadObj(const char *resourcePath, const char *objName, mem_pool *pool, model *inOutModel,
             u32 *inOutTotalFaceCount, b32 dryRun) {
	char objPath[PATH_LEN];
	StringCombine(objPath, sizeof(objPath), resourcePath, objName);
	u32 fileSize = (u32)PlatformGetFileSize(objPath);
	assert(fileSize);

	u8 *initialHiPtr = pool->hiPtr;
	char *objText = (char *)mem_push_back(pool, fileSize + 1);
	PlatformLoadFile(objPath, objText, fileSize);
	objText[fileSize] = 0;

	vec4 *vertices = 0;
	u32 nvertices = 0;

	vec2 *uvs = 0;
	u32 nuvs = 0;

	vec4 *normales = 0;
	u32 nnormales = 0;

	FaceGroup *face_groups = 0;
	u32 nface_groups = 0;
	MeshFace *faces = 0;
	u32 totalFaceCount = 0;

	material *materials = 0;
	u32 nmaterials = 0;

	tex2d *textures = 0;
	u32 ntextures = 0;

	if (!dryRun) {
		vertices = (vec4 *)mem_push(pool, inOutModel->nvertices * sizeof(*vertices));
		uvs = (vec2 *)mem_push(pool, inOutModel->nuvs * sizeof(*uvs));
		normales = (vec4 *)mem_push(pool, inOutModel->nnormales * sizeof(*normales));
		face_groups = (FaceGroup *)mem_push(pool, inOutModel->nface_groups * sizeof(*face_groups));
		faces = (MeshFace *)mem_push(pool, *inOutTotalFaceCount * sizeof(*faces));
		materials = (material *)mem_push(pool, inOutModel->nmaterials * sizeof(*materials));
		textures = (tex2d *)mem_push(pool, inOutModel->ntextures * sizeof(*textures));

		nmaterials = inOutModel->nmaterials;
	}

	for (char *text = objText; u32(text - objText) < fileSize; text += SkipLine(text)) {
		if (StringBeginsWith(text, "mtllib ")) {
			// NOTE: no support for multiple mtl filse yet
			char *localText = text + 7;
			char mtlName[PATH_LEN];
			StringCopyPred(mtlName, localText, sizeof(mtlName), StringPredCharNotInList,
			               (void *)"\n");
			u32 tc = 0;
			LoadMtl(resourcePath, mtlName, materials, &nmaterials, textures + ntextures,
			        &tc, pool, dryRun);
			ntextures += tc;
		} else if (StringBeginsWith(text, "usemtl ")) {
			// use material group
			++nface_groups;
			if (dryRun) continue;

			char *localText = text + StringLen("usemtl ");
			char materialName[RC_NAME_LEN];
			StringCopyPred(materialName, localText, sizeof(materialName), StringPredCharNotInList,
			               (void *)" \n");
			s32 materialIndex = -1;
			for (u32 i = 0; i < nmaterials; ++i) {
				if (StringCompare(materialName, materials[i].name)) {
					materialIndex = i;
					break;
				}
			}

			assert(materialIndex != -1);
			face_groups[nface_groups - 1].faces = faces + totalFaceCount;
			face_groups[nface_groups - 1].nfaces = 0;
			face_groups[nface_groups - 1].material = materials + materialIndex;
		} else if (StringBeginsWith(text, "f ")) {
			// face
			if (!nface_groups) {
				++nface_groups;
				if (!dryRun) {
					face_groups->faces = faces;
					face_groups->nfaces = 0;
					face_groups->material = 0;
				}
			}

			MeshFace face;
			char *localText = text + 2;
			for (u32 i = 0; i < 3; ++i) {
				localText += ParseUInteger(localText, &face.v[i]) + 1;
				--face.v[i];

				localText += ParseUInteger(localText, &face.uv[i]) + 1;
				--face.uv[i];

				localText += ParseUInteger(localText, &face.n[i]) + 1;
				--face.n[i];
			}

			if (!dryRun) {
				faces[totalFaceCount] = face;
				++face_groups[nface_groups - 1].nfaces;
			}

			++totalFaceCount;

			if (IsNumber(*localText)) {
				// triangulate a quad
				face.v[1] = face.v[2];
				face.uv[1] = face.uv[2];
				face.n[1] = face.n[2];
				localText += ParseUInteger(localText, &face.v[2]) + 1;
				--face.v[2];
				localText += ParseUInteger(localText, &face.uv[2]) + 1;
				--face.uv[2];
				localText += ParseUInteger(localText, &face.n[2]) + 1;
				--face.n[2];

				if (!dryRun) {
					faces[totalFaceCount] = face;
					++face_groups[nface_groups - 1].nfaces;
				}

				++totalFaceCount;
			}
		} else if (StringBeginsWith(text, "v ")) {
			// vertex
			++nvertices;
			if (dryRun) continue;

			vec4 vertice;
			ParseVector3(text + 2, &vertice);
			vertice.w = 1.0f;
			vertices[nvertices - 1] = vertice;
		} else if (StringBeginsWith(text, "vt ")) {
			// uv
			++nuvs;
			if (dryRun) continue;

			vec2 uv;
			char *localText = text + 3;
			for (u32 i = 0; i < 2; ++i) localText += ParseFloat(localText, &uv.components[i]) + 1;

			uvs[nuvs - 1] = uv;
		} else if (StringBeginsWith(text, "vn ")) {
			// normal
			++nnormales;
			if (dryRun) continue;

			vec4 normale;
			ParseVector3(text + 3, &normale);
			normale.w = 0.0f;
			normales[nnormales - 1] = normale;
		}
	}

	if (dryRun) {
		inOutModel->nvertices = nvertices;
		inOutModel->nuvs = nuvs;
		inOutModel->nnormales = nnormales;
		inOutModel->nface_groups = nface_groups;
		inOutModel->nmaterials = nmaterials;
		inOutModel->ntextures = ntextures;
		*inOutTotalFaceCount = totalFaceCount;
	} else {
		assert(inOutModel->nvertices == nvertices);
		assert(inOutModel->nuvs == nuvs);
		assert(inOutModel->nnormales == nnormales);
		assert(inOutModel->nface_groups == nface_groups);
		assert(inOutModel->nmaterials == nmaterials);
		assert(inOutModel->ntextures == ntextures);
		assert(*inOutTotalFaceCount == totalFaceCount);

		u32 facesInGroups = 0;
		for (u32 i = 0; i < nface_groups; ++i) facesInGroups += face_groups[i].nfaces;

		assert(facesInGroups == totalFaceCount);

		inOutModel->vertices = vertices;
		inOutModel->uvs = uvs;
		inOutModel->normales = normales;
		inOutModel->face_groups = face_groups;
		inOutModel->materials = materials;
		inOutModel->textures = textures;
	}

	pool->hiPtr = initialHiPtr;
}

void LoadModel(const char *resourcePath, const char *objName, mem_pool *pool, model *outModel) {
	assert(resourcePath);
	assert(objName);
	assert(pool);
	assert(outModel);

	// a dry run to collect the number of vertices, size of the textures etc.
	mem_pool initialPool = *pool;
	u32 totalFaceCount = 0;
	LoadObj(resourcePath, objName, pool, outModel, &totalFaceCount, true);
	assert(MemoryEqual(&initialPool, pool, sizeof(initialPool)));

	// an actual process of loading a model
	LoadObj(resourcePath, objName, pool, outModel, &totalFaceCount, false);
	assert(initialPool.hiPtr == pool->hiPtr);
}
