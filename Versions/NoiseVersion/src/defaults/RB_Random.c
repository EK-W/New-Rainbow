
const uint_fast32_t modulus = 0x80000000;
const uint_fast32_t multiplier = 1103515245;
const uint_fast32_t increment = 12345;

uint_fast64_t lastOutput = 0;

void RB_RandomSetSeed(uint32_t seed) {
	lastOutput = seed;
}

uint_fast32_t RB_RandomGetNext() {
	lastOutput = ((lastOutput * multiplier) + increment) % modulus;

	return (uint_fast32_t) (lastOutput & 0x7FFFFFFF);
}


