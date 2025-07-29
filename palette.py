import numpy as np

def generate_gradient(base_color, num_colors=64, specular_power=30):
    r, g, b = base_color
    
    # Create array for the gradient
    gradient = np.zeros((num_colors, 3))
    
    for i in range(num_colors):
        t = i / (num_colors - 1)
        
        # Lambert shading (linear interpolation from black to base color)
        lambert = t * np.array([r, g, b])
        
        # Specular highlight (Phong model)
        specular = 63 * np.power(t, specular_power)
        
        # Combine lambert and specular, ensuring we don't exceed the maximum value (63)
        color = np.minimum(lambert + specular, 63)
        
        gradient[i] = color
    
    # Convert to integers
    gradient = gradient.astype(int)
    
    return gradient

# Generate the gradient
maxshade = 58
base_color = (63, 48, 15)
gradient = generate_gradient(base_color, num_colors=maxshade, specular_power=8)

# Print the gradient
for i, color in enumerate(gradient):
    #print(f"Color {i}: RGB{tuple(color)}")
    r5 = color[0] >> 1
    g6 = color[1]
    b5 = color[2] >> 1
    rgb565 = (r5 << 11) | (g6 << 5) | b5
    print(f"0x{rgb565:04x}, // {tuple(color)}")
    #rhex.write(hex(color[0])[2:] + "\n")
    #ghex.write(hex(color[1])[2:] + "\n")
    #bhex.write(hex(color[2])[2:] + "\n")