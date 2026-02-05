#version 150

uniform sampler2D uTexture;
uniform vec2 uResolution;   // Screen resolution
uniform float uBlockSize;   // Size of mosaic blocks in pixels
uniform int uShape;         // 0=square, 1=hexagon, 2=triangle, 3=circle

in vec2 TexCoord;
out vec4 oColor;

// Get hexagon center
vec2 hexCenter(vec2 p, float size) {
    float s = size;
    float h = size * sqrt(3.0);

    // Axial coordinates
    float q = (2.0/3.0 * p.x) / s;
    float r = (-1.0/3.0 * p.x + sqrt(3.0)/3.0 * p.y) / s;

    // Round to nearest hex
    float x = q;
    float z = r;
    float y = -x - z;

    float rx = round(x);
    float ry = round(y);
    float rz = round(z);

    float x_diff = abs(rx - x);
    float y_diff = abs(ry - y);
    float z_diff = abs(rz - z);

    if (x_diff > y_diff && x_diff > z_diff) {
        rx = -ry - rz;
    } else if (y_diff > z_diff) {
        ry = -rx - rz;
    } else {
        rz = -rx - ry;
    }

    // Convert back to pixel coordinates
    float px = s * 3.0/2.0 * rx;
    float py = s * sqrt(3.0) * (rz + rx/2.0);

    return vec2(px, py);
}

// Get triangle center - proper equilateral triangle tessellation
vec2 triCenter(vec2 p, float size) {
    float h = size * sqrt(3.0) / 2.0;  // height of equilateral triangle

    // Which row of triangles are we in?
    float row = floor(p.y / h);

    // Offset every other row by half a triangle width
    float xOffset = mod(row, 2.0) * (size * 0.5);

    // Which column within this row?
    float adjustedX = p.x - xOffset;
    float col = floor(adjustedX / size);

    // Position within the cell
    float localX = adjustedX - col * size;
    float localY = p.y - row * h;

    // Normalized position within cell [0, 1]
    float normX = localX / size;
    float normY = localY / h;

    // Determine if we're in an upward or downward pointing triangle
    // Upward triangles: diagonal from bottom-left to top-right
    // The dividing line is: y = x (in normalized coords)
    bool isUpward = (normY < normX) != (mod(row, 2.0) == 1.0);

    vec2 center;
    if (isUpward) {
        // Upward pointing triangle - centroid at (2/3 base, 1/3 height)
        center.x = xOffset + col * size + size * 2.0 / 3.0;
        center.y = row * h + h / 3.0;
    } else {
        // Downward pointing triangle - centroid at (1/3 base, 2/3 height)
        center.x = xOffset + col * size + size / 3.0;
        center.y = row * h + h * 2.0 / 3.0;
    }

    return center;
}

void main() {
    vec2 pixelCoord = TexCoord * uResolution;
    vec2 sampleCoord;

    if (uShape == 0) {
        // Square mosaic
        vec2 blockCoord = floor(pixelCoord / uBlockSize) * uBlockSize;
        sampleCoord = (blockCoord + uBlockSize * 0.5) / uResolution;
    }
    else if (uShape == 1) {
        // Hexagonal mosaic
        vec2 center = hexCenter(pixelCoord, uBlockSize);
        sampleCoord = center / uResolution;
    }
    else if (uShape == 2) {
        // Triangle mosaic
        vec2 center = triCenter(pixelCoord, uBlockSize);
        sampleCoord = center / uResolution;
    }
    else if (uShape == 3) {
        // Circle/dot mosaic - same as square but with visible gaps
        vec2 blockCoord = floor(pixelCoord / uBlockSize) * uBlockSize;
        vec2 center = blockCoord + uBlockSize * 0.5;
        sampleCoord = center / uResolution;
    }
    else {
        // Default to square
        vec2 blockCoord = floor(pixelCoord / uBlockSize) * uBlockSize;
        sampleCoord = (blockCoord + uBlockSize * 0.5) / uResolution;
    }

    // Clamp to valid texture coordinates
    sampleCoord = clamp(sampleCoord, vec2(0.001), vec2(0.999));

    oColor = texture(uTexture, sampleCoord);
}
