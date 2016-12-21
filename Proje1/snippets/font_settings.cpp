IGUIFont *font = guienvFor2D->getFont("myfont.xml");
if (font) {
	cout << "********************* Font loaded ***************************";
} else {
	cout << "********************* Font not loaded ***************************";
}
font->draw(L"TEXT :D YAAAAAAAAY!!!!", rect<s32>(0, 0, 500, 550),SColor(255,255,255,255));