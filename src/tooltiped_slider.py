from PyQt6.QtWidgets import QSlider, QToolTip
from PyQt6.QtCore import QPoint
from PyQt6.QtGui import QMouseEvent


class TooltipedSlider(QSlider):
    def __init__(self, orientation, parent=None):
        super().__init__(orientation, parent)

    def mousePressEvent(self, event: QMouseEvent):
        super().mousePressEvent(event)
        self.show_tooltip()

    def mouseMoveEvent(self, event: QMouseEvent):
        super().mouseMoveEvent(event)
        self.show_tooltip()

    def show_tooltip(self):
        value = self.value()
        position = QPoint(self.value_to_pixel_pos(value), -25)
        QToolTip.showText(self.mapToGlobal(position), str(value), self)

    def value_to_pixel_pos(self, value):
        """Convert slider value to pixel position along the slider"""
        return int((value - self.minimum()) / (self.maximum() - self.minimum()) * self.width())
