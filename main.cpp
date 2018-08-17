#include "app.cpp"

int main(int argc, char* argv[])
{
	gst_init (&argc, &argv);
	auto app = App::create();
	const int status = app->run(argc, argv);
	
	return status;
}
