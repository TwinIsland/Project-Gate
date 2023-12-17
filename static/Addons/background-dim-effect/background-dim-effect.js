function main(backgroundId, timeInSeconds) {
    // Find the background element by its DOM ID
    var backgroundElement = document.getElementById(backgroundId);

    if (backgroundElement) {
        // Dim the background by reducing the opacity
        backgroundElement.style.transition = 'opacity 0.5s ease';
        backgroundElement.style.opacity = 0.5; // Adjust the value as needed

        // Set a timer to reset the opacity after the specified time
        setTimeout(() => {
            backgroundElement.style.opacity = 1; // Reset to full opacity
        }, timeInSeconds * 1000);
    } else {
        console.error('Background element not found with id:', backgroundId);
    }
}
