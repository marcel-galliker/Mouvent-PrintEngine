using System;
using System.ComponentModel;
using System.Diagnostics;
using System.Globalization;
using System.Linq;
using System.Resources;
using System.Text;

namespace RX_DigiPrint.Helpers
{
    /// <summary>
    /// Represents a localization manager that provides convenient access to culture-specific resources at run time.
    /// </summary>
    public class TranslationSource : INotifyPropertyChanged, ITranslationSource
    {
        #region Fields

        /// <summary>
        /// The current culture.
        /// </summary>
        private CultureInfo _currentCulture = null;

        /// <summary>
        /// The current resource manager.
        /// </summary>
        private ResourceManager _resourceManager = null;

        #endregion

        #region Events

        /// <inheritdoc/>
        public event PropertyChangedEventHandler PropertyChanged;

        #endregion

        #region Properties

        /// <summary>
        /// Gets the shared instance of the <see cref="TranslationSource"/> class.
        /// </summary>
        public static TranslationSource Instance { get; } = new TranslationSource();

        /// <inheritdoc/>
        public CultureInfo CurrentCulture
        {
            get { return this._currentCulture; }
            set
            {
                if (this._currentCulture != value)
                {
                    this._currentCulture = value;
                }
                RaisePropertyChanged();
            }
        }

        /// <inheritdoc/>
        public string this[string key]
        {
            get
            {
                string text = _resourceManager?.GetString(key, this._currentCulture);
                if (text is null)
                {
                    Debug.WriteLine($"No translation available for key = '{key}'.");
                    text = key;
                }
                return text;
            }
        }

        /// <inheritdoc/>
        public string this[string name, params object[] arguments]
        {
            get
            {
                string format = _resourceManager?.GetString(name, this._currentCulture);
                if (format is null)
                {
                    Debug.WriteLine($"No translation available for key = '{format}'.");
                    StringBuilder stringBuilder = new StringBuilder(name);
                    stringBuilder.Append(": ");
                    int idx = 0;
                    format = string.Join(" ", arguments.Select(x => stringBuilder.Append($"{{{idx++}}}")));
                }
                return string.Format(format, arguments);
            }
        }

        #endregion

        #region Methods
        public static bool has(string key)
        {
            string text = Instance._resourceManager?.GetString(key, Instance._currentCulture);
            return text != null;
        }

        /// <inheritdoc/>
        public void SetResourceManager(ResourceManager resourceManager)
        {
            if (resourceManager is null)
                throw new ArgumentNullException(nameof(resourceManager));

            _resourceManager = resourceManager;
            RaisePropertyChanged();
        }

        /// <summary>
        /// Raises the <see cref="PropertyChanged"/> event for all properties.
        /// </summary>
        private void RaisePropertyChanged()
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(string.Empty));
        }

        #endregion
    }
}
