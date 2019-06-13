import framebuf

class Ili9341Framebuffer():
    def __init__(self, display):
        self.display = display
        self.width = display.width
        self.height = display.height

    def fill(self, color = 0):
        self.display.fill(color)

    def show(self):
        pass

    def fill_rect(self, x, y, w, h, color):
        self.display.fill_rectangle(x, y, w, h, color)

    def text(self, ch, x, y, color, bgcolor = 0):
        x = min(self.width - 1, max(0, x))
        y = min(self.height - 1, max(0, y))
        w = 8
        h = 8
        buffer = bytearray(w * h * 2)
        fb = framebuf.FrameBuffer(buffer, w, h, framebuf.RGB565)
        fb.fill(bgcolor)
        fb.text(ch, 0, 0, color)
        self.display.blit_buffer(buffer, x, y, w, h)

    def scroll(self, x, y):
        pass

    def hline(self, x, y, w, color):
        self.display.hline(x, y, w, color)
