document.addEventListener('DOMContentLoaded', function() {
    // Add blob effects to the body
    addBlobEffects();
    
    // Add hero shapes if hero section exists
    addHeroShapes();
    
    // Initialize terminal effects
    initializeTerminals();
    
    // Setup animations for modules
    setupModuleAnimations();
    
    // Initialize any links with hover effects
    initializeLinks();
    
    // Initialize any CGI functionality
    initializeCGI();
});

// Add blob effects to the body
function addBlobEffects() {
    const body = document.body;
    
    // Create two blob effects and append to body
    for (let i = 0; i < 2; i++) {
        const blob = document.createElement('div');
        blob.className = 'blob-effect';
        body.prepend(blob);
        
        // Random movement animation
        setInterval(() => {
            const xPos = Math.random() * 10 - 5;
            const yPos = Math.random() * 10 - 5;
            blob.style.transform = `translate(${xPos}px, ${yPos}px)`;
        }, 2000);
    }
}

// Add hero shapes to hero section
function addHeroShapes() {
    const hero = document.querySelector('.hero');
    if (hero) {
        // Create three hero shapes and append to hero section
        for (let i = 0; i < 3; i++) {
            const shape = document.createElement('div');
            shape.className = 'hero-shape';
            hero.appendChild(shape);
        }
    }
}

// Initialize terminal effects
function initializeTerminals() {
    const terminals = document.querySelectorAll('.terminal-content');
    
    terminals.forEach(terminal => {
        const text = terminal.textContent.trim();
        if (text) {
            typeText(terminal, text);
        }
    });
}

// Function to create typing effect
function typeText(element, text, callback) {
    let index = 0;
    element.innerHTML = '';
    
    function addNextCharacter() {
        if (index < text.length) {
            const span = document.createElement('span');
            span.className = 'terminal-line';
            span.textContent = text.charAt(index);
            element.appendChild(span);
            index++;
            setTimeout(addNextCharacter, Math.random() * 50 + 30);
        } else {
            // Add cursor at the end
            const cursor = document.createElement('span');
            cursor.className = 'typed-cursor';
            element.appendChild(cursor);
            
            // Execute callback if provided
            if (typeof callback === 'function') {
                callback();
            }
        }
    }
    
    addNextCharacter();
}

// Setup animations for module cards
function setupModuleAnimations() {
    const modules = document.querySelectorAll('.module, .form-container, .feature-card');
    
    modules.forEach(module => {
        module.addEventListener('mouseenter', function() {
            this.style.transform = 'translateY(-5px)';
            this.style.boxShadow = '0 15px 35px -5px rgba(0, 0, 0, 0.3)';
        });
        
        module.addEventListener('mouseleave', function() {
            this.style.transform = 'translateY(0)';
            this.style.boxShadow = '0 10px 30px -5px rgba(0, 0, 0, 0.2)';
        });
    });
}

// Initialize links with hover effects
function initializeLinks() {
    const links = document.querySelectorAll('a:not(nav a)');
    
    links.forEach(link => {
        link.addEventListener('mouseenter', function() {
            this.style.color = 'var(--accent-main)';
        });
        
        link.addEventListener('mouseleave', function() {
            this.style.color = '';
        });
    });
}

// Initialize CGI functionality if present
function initializeCGI() {
    // Look for CGI buttons
    const cgiButtons = document.querySelectorAll('[onclick*="runCGI"]');
    
    if (cgiButtons.length > 0) {
        // Define the runCGI function if it doesn't exist
        if (typeof window.runCGI !== 'function') {
            window.runCGI = function(cgiPath) {
                const outputEl = document.getElementById("cgi-output");
                if (!outputEl) return;
                
                // Update status display with typing effect
                typeText(outputEl, "Initializing execution sequence...", () => {
                    setTimeout(() => {
                        typeText(outputEl, "Executing " + cgiPath + "...", () => {
                            // Fetch the CGI script
                            fetch(cgiPath)
                                .then(response => response.text())
                                .then(data => {
                                    let htmlContent = data;
                                    
                                    // Extract content after headers
                                    const separators = ["\r\n\r\n", "\n\n", "\r\r"];
                                    for (const separator of separators) {
                                        const index = htmlContent.indexOf(separator);
                                        if (index !== -1) {
                                            htmlContent = htmlContent.substring(index + separator.length);
                                            break;
                                        }
                                    }
                                    
                                    // Create an iframe to display the content
                                    const iframe = document.createElement('iframe');
                                    iframe.style.width = '100%';
                                    iframe.style.height = '400px';
                                    iframe.style.border = 'none';
                                    iframe.style.borderRadius = '4px';
                                    iframe.style.backgroundColor = 'transparent';
                                    iframe.scrolling = 'no';
                                    iframe.srcdoc = htmlContent;
                                    
                                    outputEl.innerHTML = "";
                                    outputEl.appendChild(iframe);
                                    
                                    iframe.onload = function() {
                                        try {
                                            iframe.style.height = (iframe.contentWindow.document.body.scrollHeight + 20) + 'px';
                                        } catch(e) {
                                            console.error("Unable to adjust iframe height:", e);
                                        }
                                    };
                                })
                                .catch(error => {
                                    typeText(outputEl, "Error: " + error.message);
                                });
                        });
                    }, 800);
                });
            };
        }
    }
    
    // File upload handling if present
    const fileInput = document.getElementById('fileInput');
    if (fileInput) {
        fileInput.addEventListener("change", function() {
            const file = this.files[0];
            const fileInfo = document.getElementById("file-info");
            
            if (file && fileInfo) {
                const previewImage = document.getElementById("preview-image");
                const noPreview = document.getElementById("no-preview");
                const fileName = document.getElementById("file-info-name");
                const fileMeta = document.getElementById("file-info-meta");
                
                fileInfo.style.display = "flex";
                fileName.textContent = file.name;
                fileMeta.textContent = `${Math.round(file.size / 1024)} KB`;
                
                if (file.type.startsWith("image/")) {
                    previewImage.src = URL.createObjectURL(file);
                    previewImage.style.display = "block";
                    noPreview.style.display = "none";
                } else {
                    previewImage.style.display = "none";
                    noPreview.textContent = "Preview not available for this file type";
                    noPreview.style.display = "block";
                }
                
                // Update status
                const uploadStatus = document.getElementById("upload-status");
                if (uploadStatus) {
                    typeText(uploadStatus, `File "${file.name}" selected and ready for upload.`);
                }
            } else if (fileInfo) {
                fileInfo.style.display = "none";
            }
        });
        
        // Add drag and drop functionality
        const dropArea = document.querySelector('.custom-file-input label');
        if (dropArea) {
            ['dragenter', 'dragover', 'dragleave', 'drop'].forEach(eventName => {
                dropArea.addEventListener(eventName, preventDefaults, false);
            });
            
            ['dragenter', 'dragover'].forEach(eventName => {
                dropArea.addEventListener(eventName, highlight, false);
            });
            
            ['dragleave', 'drop'].forEach(eventName => {
                dropArea.addEventListener(eventName, unhighlight, false);
            });
            
            dropArea.addEventListener('drop', handleDrop, false);
        }
    }
}

// Prevent default drag behaviors
function preventDefaults(e) {
    e.preventDefault();
    e.stopPropagation();
}

// Highlight drop area on drag
function highlight() {
    this.style.borderColor = 'var(--accent-main)';
    this.style.backgroundColor = 'rgba(188, 97, 243, 0.1)';
}

// Remove highlight on drop area
function unhighlight() {
    this.style.borderColor = 'rgba(188, 97, 243, 0.2)';
    this.style.backgroundColor = 'transparent';
}

// Handle file drop
function handleDrop(e) {
    const dt = e.dataTransfer;
    const files = dt.files;
    const fileInput = document.getElementById('fileInput');
    fileInput.files = files;
    
    // Trigger change event
    const event = new Event('change');
    fileInput.dispatchEvent(event);
}

// Clear upload function
function clearUpload() {
    const fileInput = document.getElementById("fileInput");
    const fileInfo = document.getElementById("file-info");
    const uploadProgress = document.getElementById("upload-progress");
    const uploadProgressFill = document.getElementById("upload-progress-fill");
    const previewImage = document.getElementById("preview-image");
    const noPreview = document.getElementById("no-preview");
    
    if (fileInput) fileInput.value = "";
    if (fileInfo) fileInfo.style.display = "none";
    if (uploadProgress) uploadProgress.style.display = "none";
    if (uploadProgressFill) {
        uploadProgressFill.style.width = "0%";
        uploadProgressFill.style.backgroundColor = ""; // Reset color
    }
    if (previewImage) previewImage.style.display = "none";
    if (noPreview) noPreview.style.display = "block";
    
    const uploadStatus = document.getElementById("upload-status");
    if (uploadStatus) {
        typeText(uploadStatus, "Upload data cleared. Ready for new operation.");
    }
}