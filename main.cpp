#include <cstdint>
#include <tuple>
import sm.flags;
import sm.vvec;
import mplot.gl.version;
import craysim.visual;

// With craysim, we must specify OpenGL 4.3 to enable Instanced VisualModels (sorry Mac users)
constexpr std::int32_t glver = mplot::gl::version_4_3;

std::int32_t main (std::int32_t argc, char* argv[])
{
    // craysim-common options parsing
    sm::flags<craysim::options> opts;
    auto[gltf_path, csv_path, h5_path, hovh] = craysim::parse_inputs (argc, argv, opts);
    // Perhaps we printed options help and can now exit
    if (opts.test (craysim::options::can_exit)) { return 1; }
    // Create a craysim main window to render the eye/sensor. This loads in the models from gltf file at path
    craysim::visual<glver> v (2000, 2000, "Compound-ray sim", gltf_path, h5_path, opts);
    // Find the model from the glTF that you want to be the landscape, then set it up
    v.find_landscape ("Landscape.003");
    v.set_hoverheight (hovh, 0.15f);
    v.speed = 3.0f; // Affects the speed of key movements
    v.setup_landscape();
    // Enable random walking. n_steps, a_tau, kappa are the params
    v.setup_random_walk (1500, 150, 100);

    // The main program loop
    while (!v.readyToFinish()) {
        v.start_loop_timer(); // It's important to call this line at the start of the loop
        v.render_and_poll(); // Does all the render computations
        // Here is where you would work on the data for the last view in v.ommatidiaData;
        v.end_loop_timer(); // Mark that we got to the end of the loop
    }
}
