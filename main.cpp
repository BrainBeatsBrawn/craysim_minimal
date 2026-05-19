import sm.flags;
import sm.vvec;
import mplot.gl.version;
import craysim.visual;

// With craysim, we must specify OpenGL 4.3 to enable Instanced VisualModels (sorry Mac users)
constexpr int glver = mplot::gl::version_4_3;

int main (int argc, char* argv[])
{
    craysim::parsed_inputs prog_opts = craysim::parse_inputs (argc, argv); // cmd-line option/file path parsing
    if (prog_opts.opts.test (craysim::options::can_exit)) { return 1; }
    // Create a craysim main window to render the eye/sensor. This loads in the models from the cmd-line specified gltf file
    craysim::visual<glver> v (2000, 2000, "Compound-ray sim", prog_opts);
    v.find_landscape ("Landscape.003"); // Find the model "Landscape.003" from the glTF and load it as the landscape
    v.set_hoverheight (prog_opts.hovh, 0.15f);
    v.kcmd_speed = 3.0f; // Affects the speed of key movements
    v.setup_landscape();
    v.setup_random_walk (1500u, 150u, 100.0f, 0.05f); // Params: n_steps, a_tau, kappa, a_max

    // The main program loop
    while (!v.readyToFinish()) {
        v.start_loop_timer(); // It's important to call this line at the start of the loop
        v.render_and_poll(); // Does all the render computations
        // Here is where you would work on the data for the last view in v.ommatidiaData;
        v.end_loop_timer(); // Mark that we got to the end of the loop
    }
}
