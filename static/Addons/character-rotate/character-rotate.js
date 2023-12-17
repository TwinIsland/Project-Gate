function main(imageId, degree) {
    // Find the image by its DOM ID
    var image = document.getElementById(imageId);

    if (image) {
        // Set the transition for smooth rotation
        image.style.transition = "transform 0.5s ease";

        // Apply the rotation
        image.style.transform = `rotate(${degree}deg)`;
    } else {
        console.error('Image not found with id:', imageId);
    }
}