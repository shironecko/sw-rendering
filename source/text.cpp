bool StringCompare(const char *a, const char *b) {
	assert(a);
	assert(b);

	while (*a && *b) {
		if (*a != *b) return false;

		++a;
		++b;
	}

	return true;
}

u32 StringLen(const char *str) {
	assert(str);
	const char *p;
	for (p = str; *p; ++p) {}

	return p - str;
}

u32 StringCopyPred(char *dest, const char *src, u32 max, bool (*pred)(char, void *),
                   void *pred_data) {
	assert(dest);
	assert(src);

	u32 i;
	for (i = 0; i < max - 1 && src[i]; ++i) {
		if (pred && !pred(src[i], pred_data)) break;

		dest[i] = src[i];
	}

	dest[i] = 0;
	return i;
}

u32 StringCopy(char *dest, const char *src, u32 max) {
	return StringCopyPred(dest, src, max, 0, 0);
}

u32 StringCat(char *dest, const char *src, u32 destLen) {
	assert(dest);
	assert(src);
	char *p = dest;
	while (*p) { ++p; }

	return p - dest + StringCopy(p, src, destLen - (p - dest));
}

void StringCombine(char *dest, u32 destLen, const char *a, const char *b) {
	assert(dest);
	assert(destLen);
	assert(a);
	assert(b);

	*dest = 0;
	StringCat(dest, a, destLen);
	StringCat(dest, b, destLen);
}

u32 SkipLine(char *inText) {
	char *text = inText;
	while (*text != 0x0A && *text != 0) { ++text; }

	return text - inText + 1;
}

bool IsNumber(char c) { return c >= '0' && c <= '9'; }

u32 ParseUInteger(char *inText, u32 *outUInt) {
	char *text = inText;
	u32 result = 0;

	while (IsNumber(*text)) {
		result *= 10;
		result += *text - '0';
		++text;
	}

	*outUInt = result;
	return text - inText;
}

u32 ParseFloat(char *inText, float *outFloat) {
	char *text = inText;
	bool positive = true;
	u32 fraction = 0;

	if (*text == '-') {
		positive = false;
		++text;
	}

	bool pointEncountered = false;
	u32 divisor = 1;
	for (;;) {
		char c = *text;
		if (IsNumber(c)) {
			fraction *= 10;
			fraction += c - '0';

			++text;

			if (pointEncountered) divisor *= 10;
		} else if (c == '.') {
			pointEncountered = true;
			++text;
		} else
			break;
	}

	float result = float(fraction) / float(divisor);
	if (!positive) result *= -1.0f;

	*outFloat = result;
	return text - inText;
}

bool StringPredCharNotInList(char c, void *d) {
	char *charList = (char *)d;
	for (char *p = charList; *p; ++p) {
		if (c == *p) return false;
	}

	return true;
}

bool StringBeginsWith(const char *string, const char *prefix) {
	while (*string && *prefix) {
		if (*string != *prefix) return false;

		++string;
		++prefix;
	}

	return true;
}
