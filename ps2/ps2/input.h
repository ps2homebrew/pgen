#ifndef _INPUT_H
#define _INPUT_H

// Main page manager class
class padManager
{
	public:

		padManager(int port, int slot);
		int updateInput();

	private:
	
		char *padBuf;
		int port, slot;
		int connected;
};

// Derived pad class, which will update the emulation input state
class ingamePadManager : public padManager
{
	public: 

		ingamePadManager(int port, int slot, t_keys *genKeys); /* : padManager(port, slot); */
		int updateEmulationInput();

	private:

		t_keys *genKeys;
};

extern padManager *guiPad;

void initPads();
void updateIngameInput();
extern "C" int isModePressed();

#endif /* _INPUT_H */
